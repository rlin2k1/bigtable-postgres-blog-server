#include <boost/asio.hpp>

#include "request.h"
#include "request_parser.h"
#include "request_handler.h"
#include "reply.h"

class session {
  public:
    session(boost::asio::io_service& io_service);
    boost::asio::ip::tcp::socket& socket();
    void start();

  private:
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    void shutdown(const boost::system::error_code& error);
    std::string get_entire_request();
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    // TODO(Michael) Change the char array into strings, so that 
    // we can get rid of the null byte that has been affecting
    // our output within our integration testing. 
    char data_[max_length+1];

    // The incoming request.
      http::server::request request_;
      // The parser for the incoming request.
      http::server::request_parser request_parser_;
    // The handler used to process the incoming request.
      http::server::request_handler request_handler_;
      // The reply to be sent back to the client.
      http::server::reply reply_;
};
