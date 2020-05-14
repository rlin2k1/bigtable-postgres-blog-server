/* session.h
Header file for handling reads of client HTTP requests and writes of responses.

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
#include "request_builder.h"
#include "request.h"
#include "request_parser.h"
#include "response.h"
#include "config_parser.h"
#include "request_dispatcher.h"
#include "response_helper_library.h"

class session {
 public:
    session(boost::asio::io_service& io_service, request_dispatcher* request_dispatcher_);
    boost::asio::ip::tcp::socket& socket();
    void start();

 private:
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    void shutdown(const boost::system::error_code& error);
    Request build_request();
    std::string get_entire_request();
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length+1];

    request_builder request_builder_;
    request_parser request_parser_;
    Response response_;

    NginxConfig* config_;
    request_dispatcher* request_dispatcher_;
};
