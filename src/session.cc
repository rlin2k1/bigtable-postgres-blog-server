/* session.cc
Handles reads of client HTTP requests and writes of replies.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "session.h"

using boost::asio::ip::tcp;

session::session(boost::asio::io_service& io_service, request_dispatcher* request_dispatcher) : socket_(io_service), request_dispatcher_(request_dispatcher) {}

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
    // After the read operation completes,
    // we call boosts::binds() -> Read() Function
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

        BOOST_LOG_TRIVIAL(info) << "Parsing request...";
        if (result == http::server::request_parser::good) {
            std::string client_http_message(request_.fullmessage.begin(), request_.fullmessage.end());

            response_ = request_dispatcher_->get_handler(request_.uri)->handle_request(request_);

            std::string log_client_message = client_http_message;

            BOOST_LOG_TRIVIAL(info) << "Parsed request successfully.";
            BOOST_LOG_TRIVIAL(info) << request_.method << " " << request_.uri
            << " HTTP/" << request_.http_version_major << "." <<
            request_.http_version_minor << " " << response_.status << " "
            << client_http_message.size();

          if (request_.keep_alive) {
              boost::asio::async_write(socket_, response_.to_buffers(),
                  boost::bind(&session::handle_write, this,
                  boost::asio::placeholders::error));
          } else {
              boost::asio::async_write(socket_, response_.to_buffers(),
                  boost::bind(&session::shutdown, this,
                  boost::asio::placeholders::error));
          }
        } else if (result == http::server::request_parser::bad) { // Return a bad request reply if request parser can't parse properly
            response_ = http::server::reply::stock_reply(http::server::reply::bad_request);
            BOOST_LOG_TRIVIAL(error) << "Request is bad. Invalid request,\
 shutting down session.";
            boost::asio::async_write(socket_, response_.to_buffers(),
                boost::bind(&session::shutdown, this,
                boost::asio::placeholders::error));
        } else {  // Keep on Reading
            memset(data_, '\0', max_length+1);
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        }
        // After the write operation completes,
        // we call boosts::binds() -> The Write Function
    } else {
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error) {
    if (!error) {
        // Asynchronously reads data FROM the stream socket TO the buffer
        BOOST_LOG_TRIVIAL(trace) << "Writing from stream socket to buffer";
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
        // After the read operation completes,
        // we call boosts::binds() -> Read() Function
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error in handle_write";
        delete this;
    }
}

void session::shutdown(const boost::system::error_code& error) {
    if (!error) {
        BOOST_LOG_TRIVIAL(info) << "Shutting down session.";
        boost::system::error_code ignored_ec;
              socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error in session shutdown." << error;
        delete this;
    }
}
