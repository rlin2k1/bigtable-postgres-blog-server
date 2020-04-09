#include <boost/asio.hpp>

// Added to prevent redefinition of class
// Can modify to ifndef/define/endif syntax if desired
#pragma once

class session {
	public:
		session(boost::asio::io_service& io_service);
		boost::asio::ip::tcp::socket& socket();
		void start();

	private:
		void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
		void handle_write(const boost::system::error_code& error);
		boost::asio::ip::tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];
};