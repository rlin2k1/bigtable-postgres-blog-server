/* server_main.cc
Main file for server creation.

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
#include <boost/thread/thread.hpp>

// TODO: After Database is implemented into the request handler, please remove
#include "blog.h"
#include "blog_database.h"
#include <pqxx/pqxx>
//------------------------------------------------------------------------------

const int THREAD_POOL_SIZE = 4;

using boost::asio::ip::tcp;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

/* Initializes logger and sets other formatting,
filtering, and file rotating information */
void logging_init() {
  logging::register_simple_formatter_factory
    < logging::trivial::severity_level, char >("Severity");

  logging::add_file_log(
    keywords::file_name = "sample_%N.log",
    // Log entries get written to immediately.
    keywords::auto_flush = true,
    // Files rotated every 10MB.
    // TODO: Add testing.
    keywords::rotation_size = 10 * 1024 * 1024,
    // Files rotated at midnight.
    // TODO: Add testing.
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

    request_dispatcher rd(config);

    server s(io_service, config.port_number, &rd);
    BOOST_LOG_TRIVIAL(info) << "Successfully started web server \
using port number "<< config.port_number;

    // TODO: After Database is implemented into the request handler, please remove
    // -------------------------------------------------------------------------- //
    // Mock Database API Example
    // -------------------------------------------------------------------------- //
    blog_database bd("postgres", "ucla", "ucla", "34.83.52.12", "5432");
    std::cout << "INSERTING BLOG POST with Title: TitlePost and Body: BodyPost and Generated PostID:" << std::endl;
    std::cout << bd.add_blog("TitlePost", "BodyPost") << std::endl;
    std::cout << "GETTING ALL BLOG POSTS IN DATABASE------------------------------------" << std::endl;
    std::vector<Blog> blogs = bd.get_all_blogs();
    for (auto vectorit = blogs.begin(); vectorit != blogs.end(); ++vectorit) {
        std::cout << "PostID = " << std::to_string((*vectorit).postid) << std::endl;
        std::cout << "Title = " << (*vectorit).title << std::endl;
        std::cout << "Body = " << (*vectorit).body << std::endl;
    }

    std::cout << "GETTING BLOG POST WITH POSTID 3------------------------------------" << std::endl;
    Blog blog = bd.get_blog(3);
    std::cout << "PostID = " << std::to_string(blog.postid) << std::endl;
    std::cout << "Title = " << blog.title << std::endl;
    std::cout << "Body = " << blog.body << std::endl;

    // End of Mock Database API Example
    //----------------------------------------------------------------------------------------------

    // Create a pool of threads to run all of the io_services.
    std::vector<boost::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i)
    {
      boost::shared_ptr<boost::thread> thread(new boost::thread(
            boost::bind(&boost::asio::io_service::run, &io_service)));
      threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
      threads[i]->join();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
