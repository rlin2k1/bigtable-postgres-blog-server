/* session.cc
Handles reads of client HTTP requests and writes of responses.

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

/* Read and parses requests recieved, and if success, provided response for handle_write to read. */
void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        // Asynchronously writes to the socket_ everything in the buffer.
        request_parser::result_type result;
        std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data_, data_ + bytes_transferred);

        BOOST_LOG_TRIVIAL(info) << "Parsing request...";
        if (result == request_parser::good) {
            Request req = request_builder_.build_request();
            response_ = request_dispatcher_->get_handler(req.uri_)->handle_request(req);

            if (request_dispatcher_->status_handler_enabled){
                BOOST_LOG_TRIVIAL(info) << "Status handler enabled, recording request.";
                request_dispatcher_->get_status_handler()->record_received_request(req.uri_, response_.code_);
            }

            BOOST_LOG_TRIVIAL(info) << "Parsed request successfully.";
            BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]RequestPath: " << req.uri_;
            BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]ResponseCode: " << response_.code_;
            BOOST_LOG_TRIVIAL(info) << req.method_ << " " << req.uri_ << " "
            << req.version_ << " " << response_.code_ << " "
            << request_builder_.fullmessage.size();



          if (request_builder_.keep_alive) {
              request_parser_.reset();
              request_builder_ = request_builder();

              boost::asio::async_write(socket_, ResponseHelperLibrary::to_buffers(response_),
                  boost::bind(&session::handle_write, this,
                  boost::asio::placeholders::error));
          } else {
              boost::asio::async_write(socket_, ResponseHelperLibrary::to_buffers(response_),
                  boost::bind(&session::shutdown, this,
                  boost::asio::placeholders::error));
          }
        } else if (result == request_parser::bad) {  // Return a bad request Response if request parser can't parse properly
            response_ = ResponseHelperLibrary::stock_response(Response::bad_request);
            BOOST_LOG_TRIVIAL(error) << "Request is bad. Invalid request,\
 shutting down session.";
            BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]Request_Handler: 400";
            BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]RequestPath: 400";
            BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]ResponseCode: 400";
            boost::asio::async_write(socket_, ResponseHelperLibrary::to_buffers(response_),
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

/* Writes data from handle_read to the buffer. */
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
