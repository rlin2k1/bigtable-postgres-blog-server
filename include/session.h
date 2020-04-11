#include <boost/asio.hpp>

#include "request.h"
#include "request_parser.h"

class session {
	public:
		session(boost::asio::io_service& io_service);
		boost::asio::ip::tcp::socket& socket();
		void start();

	private:
		void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
		void handle_write(const boost::system::error_code& error);
		void shutdown(const boost::system::error_code& error);
		boost::asio::ip::tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];

		// The incoming request.
  		http::server::request request_;
  		// The parser for the incoming request.
  		http::server::request_parser request_parser_;
};