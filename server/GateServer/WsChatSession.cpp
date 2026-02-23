#include "WsChatSession.h"
#include <algorithm>
#include <atomic>

namespace {
std::atomic<unsigned long long> g_guest_id{ 1 };

std::string Trim(const std::string& value) {
	const auto begin = value.find_first_not_of(" \r\n\t");
	if (begin == std::string::npos) {
		return "";
	}

	const auto end = value.find_last_not_of(" \r\n\t");
	return value.substr(begin, end - begin + 1);
}
}  // namespace

void WsChatRoom::Join(const std::shared_ptr<WsChatSession>& session) {
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.emplace_back(session);
}

void WsChatRoom::Leave(const std::shared_ptr<WsChatSession>& session) {
	const auto raw = session.get();
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.erase(
		std::remove_if(_sessions.begin(), _sessions.end(),
			[raw](const std::weak_ptr<WsChatSession>& weak) {
				auto current = weak.lock();
				return !current || current.get() == raw;
			}),
		_sessions.end());
}

void WsChatRoom::Broadcast(const std::string& message) {
	std::vector<std::shared_ptr<WsChatSession>> targets;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		for (auto it = _sessions.begin(); it != _sessions.end();) {
			auto current = it->lock();
			if (!current) {
				it = _sessions.erase(it);
				continue;
			}
			targets.push_back(current);
			++it;
		}
	}

	for (const auto& session : targets) {
		session->Deliver(message);
	}
}

WsChatSession::WsChatSession(tcp::socket socket)
	: _ws(std::move(socket)),
	_nickname("guest-" + std::to_string(g_guest_id.fetch_add(1))) {
}

void WsChatSession::Run(http::request<http::dynamic_body> req) {
	_ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
	_ws.set_option(websocket::stream_base::decorator(
		[](websocket::response_type& response) {
			response.set(http::field::server, "GateServer-WsChat");
		}));

	auto self = shared_from_this();
	_ws.async_accept(
		req,
		[self](beast::error_code ec) {
			self->OnAccept(ec);
		});
}

void WsChatSession::Deliver(const std::string& message) {
	auto self = shared_from_this();
	net::post(
		_ws.get_executor(),
		[self, message]() {
			const bool writing = !self->_send_queue.empty();
			self->_send_queue.push_back(message);
			if (!writing) {
				self->DoWrite();
			}
		});
}

void WsChatSession::OnAccept(beast::error_code ec) {
	if (ec) {
		std::cout << "WebSocket accept error: " << ec.message() << std::endl;
		return;
	}

	WsChatRoom::GetInstance()->Join(shared_from_this());
	WsChatRoom::GetInstance()->Broadcast("[system] " + _nickname + " joined");
	Deliver("[system] connected. Use /nick your_name to rename.");
	DoRead();
}

void WsChatSession::DoRead() {
	auto self = shared_from_this();
	_ws.async_read(
		_buffer,
		[self](beast::error_code ec, std::size_t bytes_transferred) {
			self->OnRead(ec, bytes_transferred);
		});
}

void WsChatSession::OnRead(beast::error_code ec, std::size_t bytes_transferred) {
	boost::ignore_unused(bytes_transferred);
	if (ec == websocket::error::closed) {
		LeaveRoom();
		return;
	}

	if (ec) {
		std::cout << "WebSocket read error: " << ec.message() << std::endl;
		LeaveRoom();
		return;
	}

	const std::string message = Trim(beast::buffers_to_string(_buffer.data()));
	_buffer.consume(_buffer.size());
	HandleMessage(message);
	DoRead();
}

void WsChatSession::DoWrite() {
	auto self = shared_from_this();
	_ws.text(true);
	_ws.async_write(
		net::buffer(_send_queue.front()),
		[self](beast::error_code ec, std::size_t bytes_transferred) {
			self->OnWrite(ec, bytes_transferred);
		});
}

void WsChatSession::OnWrite(beast::error_code ec, std::size_t bytes_transferred) {
	boost::ignore_unused(bytes_transferred);
	if (ec) {
		std::cout << "WebSocket write error: " << ec.message() << std::endl;
		LeaveRoom();
		return;
	}

	_send_queue.pop_front();
	if (!_send_queue.empty()) {
		DoWrite();
	}
}

void WsChatSession::HandleMessage(const std::string& message) {
	if (message.empty()) {
		return;
	}

	if (message.compare(0, 6, "/nick ") == 0) {
		const std::string updated_name = Trim(message.substr(6));
		if (updated_name.empty()) {
			Deliver("[system] nickname cannot be empty");
			return;
		}

		const std::string old_name = _nickname;
		_nickname = updated_name;
		WsChatRoom::GetInstance()->Broadcast("[system] " + old_name + " is now " + _nickname);
		return;
	}

	WsChatRoom::GetInstance()->Broadcast("[" + _nickname + "] " + message);
}

void WsChatSession::LeaveRoom() {
	WsChatRoom::GetInstance()->Leave(shared_from_this());
	WsChatRoom::GetInstance()->Broadcast("[system] " + _nickname + " left");
}
