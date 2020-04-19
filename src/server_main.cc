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

using boost::asio::ip::tcp;

void handle_sigterm(int sig) {
    exit(1);
}

int main(int argc, char* argv[]) {
  signal(SIGTERM, handle_sigterm);

  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <config_file>\n";
      return 1;
    }

    NginxConfigParser config_parser;
    NginxConfig config;
    config_parser.Parse(argv[1], &config);

    boost::asio::io_service io_service;

    if (config.port_number < 0) {
      std::cerr << "Could not find valid port number in config" << std::endl;
      return 1;
    }

    server s(io_service, config.port_number);

    io_service.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
