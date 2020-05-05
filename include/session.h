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

class session {
 public:
    session(boost::asio::io_service& io_service, NginxConfig* config);
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

    // The incoming request.
    http::server::request request_;
    // The parser for the incoming request.
    http::server::request_parser request_parser_;
    // The handler used to process the incoming request.
    http::server::echo_request_handler echo_request_handler_;
    // Handler used to parse files.
    http::server::static_request_handler static_request_handler_;
    // The reply to be sent back to the client.
    http::server::reply reply_;
    NginxConfig* config_;
};
