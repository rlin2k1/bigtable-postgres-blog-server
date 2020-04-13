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
	// Asynchronously reads data FROM the stream socket TO the buffer
	memset(data_, '\0', max_length+1);
	socket_.async_read_some(boost::asio::buffer(data_, max_length),
							boost::bind(&session::handle_read, this,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
	// After the read operation completes, we call boosts::binds() -> Read() Function
	/* session::handle_read and session::handle_write
	   go back and forth calling each other!
	*/
}


void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
	if (!error) {
		// Asynchronously writes to the socket_ everything in the buffer. 
		http::server::request_parser::result_type result;
		std::tie(result, std::ignore) = request_parser_.parse(
              request_, data_, data_ + bytes_transferred);

		if (result == http::server::request_parser::good) {
			std::string client_http_message = get_entire_request();
			request_handler_.handle_request(request_, reply_, client_http_message.c_str());

            boost::asio::async_write(socket_, reply_.to_buffers(),
				boost::bind(&session::handle_write, this,
				boost::asio::placeholders::error));
        } else if (result == http::server::request_parser::bad) {
			reply_ = http::server::reply::stock_reply(http::server::reply::bad_request);

            boost::asio::async_write(socket_, reply_.to_buffers(),
				boost::bind(&session::shutdown, this,
				boost::asio::placeholders::error));
        } else { // Keep on Reading
            socket_.async_read_some(boost::asio::buffer(data_, max_length), 
				boost::bind(&session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
        }
		// After the write operation completes, we call boosts::binds() -> The Write Function
	} else {
		delete this;
	}
}

void session::handle_write(const boost::system::error_code& error) {
	if (!error) {
		// Asynchronously reads data FROM the stream socket TO the buffer
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			boost::bind(&session::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
		// After the read operation completes, we call boosts::binds() -> Read() Function
	} else {
		delete this;
	}
}

void session::shutdown(const boost::system::error_code& error) {
	if (!error) {
		boost::system::error_code ignored_ec;
          	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
	} else {
		delete this;
	}
}

std::string session::get_entire_request() {
	std::string data_string(data_);
	int divider_position = data_string.find("\r\n\r\n");
	std::string header = data_string.substr(0, divider_position);
	std::string body = data_string.substr(divider_position);
	// keep reading while there are still bytes unread in the socket
	while (socket_.available()) {
		std::vector<char> tempbuf(max_length);
		socket_.read_some(boost::asio::buffer(tempbuf, max_length));
		std::string body_addition(tempbuf.begin(), tempbuf.end());
		body = body + body_addition;
	}
	return header + body;
}
