#ifndef LOGIN_MESSAGE_HANDLE_HPP
#define LOGIN_MESSAGE_HANDLE_HPP

#include <boost/beast/core.hpp>
#include <boost/json.hpp>
#include <memory>
#include <regex>

#include "../server/websocket_server.hpp"
#include "message_handle.hpp"
#include "../dal/user_dao.hpp"

class LoginMessageHandle : public MessageHandle, public std::enable_shared_from_this<LoginMessageHandle> {
public:
    LoginMessageHandle(const Message& message) : MessageHandle(message) {}

    void handle(std::shared_ptr<WebSocketSession> session,
                std::unique_ptr<MessageParse> parsed_message) override {
        std::string response_message;

        auto [account, password] = parseJson(parsed_message->getContent());
        // 验证账户和密码格式
        response_message = validateAndLogin(account, password);

        // 发送响应回 WebSocket 客户端
        sendResponse(session, response_message);
    }

private:
    // 解析 JSON 并提取账户和密码
    std::pair<std::string, std::string> parseJson(const std::string& content) {
        boost::json::value jsonValue = boost::json::parse(content);
        auto jsonObject = jsonValue.as_object();

        if (!jsonObject.contains("account") || !jsonObject.contains("password")) {
            throw std::invalid_argument("Missing account or password in content");
        }

        std::string account = jsonObject["account"].as_string().c_str();
        std::string password = jsonObject["password"].as_string().c_str();
        return {account, password};
    }

    // 验证账户和密码格式并进行登录
    std::string validateAndLogin(const std::string& account,
                                 const std::string& password) {
        // 定义正则表达式进行验证
        std::regex account_regex("^[A-Za-z]{10}$");  // 账户必须是10个字母
        std::regex password_regex("^[0-9]{10}$");  // 密码必须是10个数字

        if (!std::regex_match(account, account_regex) ||
            !std::regex_match(password, password_regex)) {
            return generateErrorResponse(account, password);
        } else {
            // 调用 UserDAO::Login 函数验证用户
            std::string result = UserDAO::Login(account, password);
            return generateResponse(result);
        }
    }

    // 生成错误响应
    std::string generateErrorResponse(const std::string& account,
                                      const std::string& password) {
        std::string error_message = R"({"status":"error", "message":")";
        error_message +=
            !std::regex_match(account, std::regex("^[A-Za-z]{10}$"))
                ? "Account must contain only 10 letters"
                : "Password must contain only 10 digits";
        error_message += R"("})";
        return error_message;
    }

    // 生成响应
    std::string generateResponse(const std::string& result) {
        if (result == "Login successful") {
            return R"({"status":"success", "message":"Login successful"})";
        } else if (result == "Invalid credentials") {
            return R"({"status":"error", "message":"Invalid account or password"})";
        } else if (result == "User does not exist") { // 添加用户不存在的情况
            return R"({"status":"error", "message":"User does not exist"})";
        } else {
            return R"({"status":"error", "message":"Login failed"})";
        }
    }

    // 发送响应回 WebSocket 客户端
    void sendResponse(std::shared_ptr<WebSocketSession> session,
                      const std::string& response_message) {
        auto buffer = net::buffer(response_message);
        session->getWebSocketStream().async_write(
            buffer,
            [self = shared_from_this()](beast::error_code ec,
                                        std::size_t bytes_transferred) {
                if (ec) {
                    LOG(Level::ERROR, "Write failed: ", ec.message());
                    return;
                }
                LOG(Level::INFO, "Sent message: ", bytes_transferred, " bytes");
            });
    }
};

#endif  // LOGIN_MESSAGE_HANDLE_HPP
