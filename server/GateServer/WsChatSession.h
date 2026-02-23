#pragma once

#include "const.h"
#include <deque>
#include <mutex>
#include <vector>

class WsChatSession;

class WsChatRoom : public Singleton<WsChatRoom> {
	friend class Singleton<WsChatRoom>;
public:
	void Join(const std::shared_ptr<WsChatSession>& session);
	void Leave(const std::shared_ptr<WsChatSession>& session);
	void Broadcast(const std::string& message);

private:
	WsChatRoom() = default;

	std::mutex _mutex;
	std::vector<std::weak_ptr<WsChatSession>> _sessions;
};

class WsChatSession : public std::enable_shared_from_this<WsChatSession> {
public:
	explicit WsChatSession(tcp::socket socket);
	void Run(http::request<http::dynamic_body> req);
	void Deliver(const std::string& message);

private:
	void OnAccept(beast::error_code ec);
	void DoRead();
	void OnRead(beast::error_code ec, std::size_t bytes_transferred);
	void DoWrite();
	void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
	void HandleMessage(const std::string& message);
	void LeaveRoom();

	websocket::stream<tcp::socket> _ws;
	beast::flat_buffer _buffer;
	std::deque<std::string> _send_queue;
	std::string _nickname;
};
