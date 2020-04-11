#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "session.h"

using boost::asio::ip::tcp;

session::session(boost::asio::io_service& io_service) : socket_(io_service) {}

tcp::socket& session::socket() {
	return socket_;
}

void session::start() {
	socket_.async_read_some(boost::asio::buffer(data_, max_length),
							boost::bind(&session::handle_read, this,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
}


void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
	if (!error) {
		// Asynchronously writes to the socket_ everything in the buffer. 
		std::string resp = "HTTP/1.1 200 OK/\r\nContent-Length: " + std::to_string(bytes_transferred) + "\r\nContent-Type: text/plain\r\n\r\n";

		int buffer_length = resp.length() + bytes_transferred;

		boost::asio::async_write(socket_,
			boost::asio::buffer(resp + data_, buffer_length),
			boost::bind(&session::handle_write, this,
			boost::asio::placeholders::error));
	} else {
		delete this;
	}
}

void session::handle_write(const boost::system::error_code& error) {
	if (!error) {
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			boost::bind(&session::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	} else {
		delete this;
	}
}
