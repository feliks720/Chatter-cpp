#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include <random>
#include <sstream>

namespace {
bool ParseRequestBody(const std::shared_ptr<HttpConnection>& connection, Json::Value& out) {
	const auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
	Json::CharReaderBuilder builder;
	std::string errs;
	std::istringstream stream(body_str);
	return Json::parseFromStream(builder, stream, &out, &errs);
}

void WriteJson(const std::shared_ptr<HttpConnection>& connection, const Json::Value& root) {
	connection->_response.set(http::field::content_type, "application/json");
	beast::ostream(connection->_response.body()) << root.toStyledString();
}
}  // namespace

LogicSystem::LogicSystem() {
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req " << std::endl;
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " <<  " value is " << elem.second << std::endl;
		}
		return true;
	});

	const auto get_code_handler = [this](std::shared_ptr<HttpConnection> connection) {
		Json::Value root;
		Json::Value request_json;
		const bool parse_success = ParseRequestBody(connection, request_json);
		if (!parse_success || !request_json.isObject() || !request_json["email"].isString()) {
			root["error"] = ErrorCodes::Error_Json;
			WriteJson(connection, root);
			return true;
		}

		const std::string email = request_json["email"].asString();
		if (email.empty()) {
			root["error"] = ErrorCodes::Error_InvalidParams;
			root["message"] = "email is empty";
			WriteJson(connection, root);
			return true;
		}

		GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
		root["error"] = rsp.error();
		root["email"] = email;

		if (rsp.error() == ErrorCodes::Success) {
			std::string code = rsp.code();
			if (code.empty()) {
				code = GenerateVerifyCode();
			}

			{
				std::lock_guard<std::mutex> lock(_mutex);
				CleanupExpiredCodesLocked();
				_pending_codes[email] = PendingCode{
					code,
					std::chrono::steady_clock::now() + std::chrono::minutes(3)
				};
			}

			// Keep code in response for local development/testing convenience.
			root["code"] = code;
		}

		WriteJson(connection, root);
		return true;
	};

	RegPost("/get_varifycode", get_code_handler);
	RegPost("/get_verifycode", get_code_handler);

	RegPost("/register", [this](std::shared_ptr<HttpConnection> connection) {
		Json::Value root;
		Json::Value request_json;
		const bool parse_success = ParseRequestBody(connection, request_json);
		if (!parse_success || !request_json.isObject()) {
			root["error"] = ErrorCodes::Error_Json;
			WriteJson(connection, root);
			return true;
		}

		const std::string user = request_json.get("user", "").asString();
		const std::string email = request_json.get("email", "").asString();
		const std::string password = request_json.get("password", "").asString();
		const std::string code = request_json.get("code", "").asString();

		if (user.empty() || email.empty() || password.empty() || code.empty()) {
			root["error"] = ErrorCodes::Error_InvalidParams;
			root["message"] = "required field is empty";
			WriteJson(connection, root);
			return true;
		}

		{
			std::lock_guard<std::mutex> lock(_mutex);
			CleanupExpiredCodesLocked();

			auto code_it = _pending_codes.find(email);
			if (code_it == _pending_codes.end()) {
				root["error"] = ErrorCodes::Error_CodeExpired;
				root["message"] = "verify code expired";
				WriteJson(connection, root);
				return true;
			}

			if (code_it->second.code != code) {
				root["error"] = ErrorCodes::Error_CodeMismatch;
				root["message"] = "verify code mismatch";
				WriteJson(connection, root);
				return true;
			}

			if (_users.find(user) != _users.end() || _email_to_user.find(email) != _email_to_user.end()) {
				root["error"] = ErrorCodes::Error_UserExists;
				root["message"] = "user already exists";
				WriteJson(connection, root);
				return true;
			}

			_users[user] = UserInfo{ email, password };
			_email_to_user[email] = user;
			_pending_codes.erase(code_it);
		}

		root["error"] = ErrorCodes::Success;
		root["user"] = user;
		root["message"] = "register success";
		WriteJson(connection, root);
		return true;
	});

	RegPost("/login", [this](std::shared_ptr<HttpConnection> connection) {
		Json::Value root;
		Json::Value request_json;
		const bool parse_success = ParseRequestBody(connection, request_json);
		if (!parse_success || !request_json.isObject()) {
			root["error"] = ErrorCodes::Error_Json;
			WriteJson(connection, root);
			return true;
		}

		const std::string user = request_json.get("user", "").asString();
		const std::string password = request_json.get("password", "").asString();
		if (user.empty() || password.empty()) {
			root["error"] = ErrorCodes::Error_InvalidParams;
			root["message"] = "required field is empty";
			WriteJson(connection, root);
			return true;
		}

		{
			std::lock_guard<std::mutex> lock(_mutex);
			const auto user_it = _users.find(user);
			if (user_it == _users.end()) {
				root["error"] = ErrorCodes::Error_UserNotFound;
				root["message"] = "user not found";
				WriteJson(connection, root);
				return true;
			}

			if (user_it->second.password != password) {
				root["error"] = ErrorCodes::Error_PasswordMismatch;
				root["message"] = "password mismatch";
				WriteJson(connection, root);
				return true;
			}

			root["error"] = ErrorCodes::Success;
			root["user"] = user;
			root["email"] = user_it->second.email;
			root["message"] = "login success";
		}

		WriteJson(connection, root);
		return true;
	});
}

void LogicSystem::RegGet(std::string url, HttpHandler handler) {
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler) {
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::~LogicSystem() {

}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) {
	const auto it = _get_handlers.find(path);
	if (it == _get_handlers.end()) {
		return false;
	}

	return it->second(con);
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con) {
	const auto it = _post_handlers.find(path);
	if (it == _post_handlers.end()) {
		return false;
	}

	return it->second(con);
}

void LogicSystem::CleanupExpiredCodesLocked() {
	const auto now = std::chrono::steady_clock::now();
	for (auto it = _pending_codes.begin(); it != _pending_codes.end();) {
		if (it->second.expire_at <= now) {
			it = _pending_codes.erase(it);
			continue;
		}
		++it;
	}
}

std::string LogicSystem::GenerateVerifyCode() {
	thread_local std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(0, 9);

	std::string code;
	code.reserve(6);
	for (int i = 0; i < 6; ++i) {
		code.push_back(static_cast<char>('0' + dist(rng)));
	}
	return code;
}