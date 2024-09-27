#pragma once
#include <boost/beast.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <memory>
#include "../bll/message_paraser.hpp"

namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
   public:
    WebSocketSession(tcp::socket socket);

    void start();
    websocket::stream<tcp::socket>& getWebSocketStream();
    bool isLogin();
    void setLogin();

   private:
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    bool is_login;

    void on_accept(beast::error_code ec);
    void read_message();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void handle_message(const std::string& message);
    void checkLoginStatus(MessageType message_type);
};

class WebSocketServer {
   public:
    WebSocketServer(net::io_context& ioc, tcp::endpoint endpoint);

   private:
    tcp::acceptor acceptor_;

    void do_accept();
};
