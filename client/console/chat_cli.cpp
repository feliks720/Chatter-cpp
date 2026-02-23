#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main(int argc, char* argv[]) {
    const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
    const std::string port = argc > 2 ? argv[2] : "8080";
    const std::string path = argc > 3 ? argv[3] : "/ws";

    try {
        net::io_context ioc;
        tcp::resolver resolver(ioc);
        websocket::stream<tcp::socket> ws(ioc);

        const auto results = resolver.resolve(host, port);
        net::connect(ws.next_layer(), results.begin(), results.end());
        ws.handshake(host + ":" + port, path);

        std::cout << "Connected to ws://" << host << ":" << port << path << '\n';
        std::cout << "Type /nick your_name to change nickname, /quit to exit." << '\n';

        std::atomic<bool> running{ true };
        std::thread reader([&]() {
            while (running.load()) {
                beast::flat_buffer buffer;
                beast::error_code ec;
                ws.read(buffer, ec);
                if (ec) {
                    if (ec != websocket::error::closed) {
                        std::cerr << "Read error: " << ec.message() << '\n';
                    }
                    running.store(false);
                    return;
                }
                std::cout << beast::buffers_to_string(buffer.data()) << '\n';
            }
        });

        std::string line;
        while (running.load() && std::getline(std::cin, line)) {
            if (line == "/quit") {
                running.store(false);
                break;
            }

            beast::error_code ec;
            ws.write(net::buffer(line), ec);
            if (ec) {
                std::cerr << "Write error: " << ec.message() << '\n';
                running.store(false);
                break;
            }
        }

        beast::error_code ec;
        ws.close(websocket::close_code::normal, ec);
        if (ec && ec != websocket::error::closed) {
            std::cerr << "Close error: " << ec.message() << '\n';
        }

        running.store(false);
        if (reader.joinable()) {
            reader.join();
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
