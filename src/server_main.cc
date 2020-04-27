//
// server_main.cc
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <signal.h>
#include <stdio.h>

#include "server.h"
#include "session.h"
#include "config_parser.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

using boost::asio::ip::tcp;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void logging_init() {
  logging::register_simple_formatter_factory
    < logging::trivial::severity_level, char >("Severity");

  logging::add_file_log(
    // TODO(janejiwonlee): See if file name pattern
    // needs to have specific naming convention.
    keywords::file_name = "sample_%N.log",
    // Log entries get written to immediately.
    keywords::auto_flush = true,
    // Files rotated every 10MB.
    // TODO(janejiwonlee): Add testing.
    keywords::rotation_size = 10 * 1024 * 1024,
    // Files rotated at midnight.
    // TODO(janejiwonlee): Add testing.
    keywords::time_based_rotation = sinks::file::
      rotation_at_time_point(0, 0, 0),
    keywords::format = "%TimeStamp% | %ThreadID% | %Severity% | %Message%");

  logging::add_console_log(
    std::cout,
    keywords::auto_flush = true,
    keywords::format = "%TimeStamp% | %ThreadID% | %Severity% | %Message%"),
    // Appends to existing file.
    keywords::open_mode = std::ios_base::app;

  // Setting a filter to log from severity level info to above.
  logging::core::get()->set_filter(
    logging::trivial::severity >= logging::trivial::info);
}

void handle_sigterm(int sig) {
  BOOST_LOG_TRIVIAL(info) << "Program terminated with SIGTERM";
  exit(1);
}

void handle_sigint(int sig) {
  BOOST_LOG_TRIVIAL(info) << "Program terminated with SIGINT";
  exit(1);
}

int main(int argc, char* argv[]) {
  logging_init();
  logging::add_common_attributes();   // LineID, TimeStamp, ProcessID, ThreadID


  signal(SIGTERM, handle_sigterm);
  signal(SIGINT, handle_sigint);

  try {
    if (argc != 2) {
      BOOST_LOG_TRIVIAL(error) << "Usage: async_tcp_echo_server \
<config_file>\n";
      return 1;
    }

    NginxConfigParser config_parser;
    NginxConfig config;
    config_parser.Parse(argv[1], &config);

    boost::asio::io_service io_service;

    if (config.port_number < 0) {
      BOOST_LOG_TRIVIAL(error) << "Could not find valid port number in config";
      return 1;
    }

    server s(io_service, &config);
    BOOST_LOG_TRIVIAL(info) << "Successfully started web server \
using port number "<< config.port_number;

    io_service.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
