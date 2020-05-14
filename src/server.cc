/* server.cc
Creates servers and handles connections from clients.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 8th, 2020
*/

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "server.h"
#include "session.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/current_process_id.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;

using boost::asio::ip::tcp;

server::server(boost::asio::io_service& io_service, int port_number, request_dispatcher* request_dispatcher) :
                                io_service_(io_service), request_dispatcher_(request_dispatcher), acceptor_(io_service,
                                tcp::endpoint(tcp::v4(), port_number)) {

        BOOST_LOG_TRIVIAL(info) << "ProcessID of server is: " << getpid();

        start_accept();
}

//  Accepts new session created.
void server::start_accept() {
    session* new_session = new session(io_service_, request_dispatcher_);
    acceptor_.async_accept(new_session->socket(),
                                                boost::bind(&server::handle_accept,
                                                this, new_session,
                                                boost::asio::placeholders::error));
}

//  Creates new session for the user.
void server::handle_accept(session* new_session, const boost::system::error_code& error) {
        if (!error)	{
                boost::system::error_code ec;
                boost::asio::ip::tcp::endpoint endpoint = new_session->socket().remote_endpoint(ec);
                BOOST_LOG_TRIVIAL(info) << "Successfully started new session.";
                BOOST_LOG_TRIVIAL(info) << "Client at IP Address: " << endpoint;

        new_session->start();
    } else {
                BOOST_LOG_TRIVIAL(error) << "Error accepting session.";
        delete new_session;
    }
    start_accept();
}
