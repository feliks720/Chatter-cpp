#pragma once
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include "const.h"

class HttpConnection;
typedef std::function<bool(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem :public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
	void RegGet(std::string, HttpHandler handler);
	void RegPost(std::string, HttpHandler handler);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
private:
	struct PendingCode {
		std::string code;
		std::chrono::steady_clock::time_point expire_at;
	};

	struct UserInfo {
		std::string email;
		std::string password;
	};

	void CleanupExpiredCodesLocked();
	std::string GenerateVerifyCode();
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
	std::mutex _mutex;
	std::unordered_map<std::string, PendingCode> _pending_codes;
	std::unordered_map<std::string, UserInfo> _users;
	std::unordered_map<std::string, std::string> _email_to_user;
};

