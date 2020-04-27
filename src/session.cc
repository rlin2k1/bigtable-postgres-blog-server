#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include "session.h"

#define DIR_INDEX 1

using boost::asio::ip::tcp;

session::session(boost::asio::io_service& io_service, NginxConfig* config) : socket_(io_service), config_(config) {}

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
        if (result == http::server::request_parser::good) {
            std::string client_http_message(request_.fullmessage.begin(), request_.fullmessage.end());

            // Find the root directory and target file from the client's request uri
            std::vector<std::string> path_elements;
            boost::split(path_elements, request_.uri, boost::is_any_of("/"));
            std::string target_dir = "/" + path_elements[DIR_INDEX];
            std::string target_file = path_elements[path_elements.size() - 1];

            // Rebuild the uri without the root folder, which is added in the request handler
            std::string partial_uri;
            for (int i = DIR_INDEX + 1; i < path_elements.size(); i++) {
                partial_uri = partial_uri + "/" + path_elements[i];
            }

            // Call the corresponding handler to handle the request
            if (config_->echo_locations_.find(request_.uri) != config_->echo_locations_.end()) {
                echo_request_handler_.handle_request(request_, reply_, client_http_message.c_str());
            } else if (config_->static_locations_.find(target_dir) != config_->static_locations_.end()) {
                static_request_handler_.config_ = config_;
                static_request_handler_.target_dir_ = target_dir;
                static_request_handler_.target_file_ = target_file;
                static_request_handler_.partial_uri_ = partial_uri;
                static_request_handler_.handle_request(request_, reply_, client_http_message.c_str());
            } else { // TODO: Should probably make a default bad case handler
                static_request_handler_.default_handle_bad_request(reply_);
            }

          if (request_.keep_alive) {
              boost::asio::async_write(socket_, reply_.to_buffers(),
                  boost::bind(&session::handle_write, this,
                  boost::asio::placeholders::error));
          } else {
              boost::asio::async_write(socket_, reply_.to_buffers(),
                  boost::bind(&session::shutdown, this,
                  boost::asio::placeholders::error));
          }
        } else if (result == http::server::request_parser::bad) {
            reply_ = http::server::reply::stock_reply(http::server::reply::bad_request);

            boost::asio::async_write(socket_, reply_.to_buffers(),
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
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
        // After the read operation completes,
        // we call boosts::binds() -> Read() Function
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
