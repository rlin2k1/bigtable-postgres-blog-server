/* session.h
Header file for handling reads of client HTTP requests and writes of replies.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 8th, 2020
*/

#include <boost/asio.hpp>
#include <string>
#include "request.h"
#include "request_parser.h"
#include "reply.h"
#include "request_handler.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"
#include "config_parser.h"
#include "request_dispatcher.h"

class session {
 public:
    session(boost::asio::io_service& io_service, request_dispatcher* request_dispatcher_);
    boost::asio::ip::tcp::socket& socket();
    void start();

 private:
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    void shutdown(const boost::system::error_code& error);
    std::string get_entire_request();
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length+1];

    http::server::request request_;
    http::server::request_parser request_parser_;
    http::server::reply reply_;

    NginxConfig* config_;
    request_dispatcher* request_dispatcher_;
};
