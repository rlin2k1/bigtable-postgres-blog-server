/* NginxConfigParser.cc
Description:
  Parser for nginx configuration files.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 9th, 2020
*/

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "config_parser.h"

#define MAX_PORT 65535
#define BLOG_INFO_FIELDS 4

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
    case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
    case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
    default:                       return "Unknown token type";
  }
}

/* NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input, std::string* value)
  Parameter(s):
    - input: Input stream, coming from the configuration file.
    - value: Stores the value of the current token being parsed.
  Returns:
    - Enum TokenType which helps determine the state of the parser. (See onfig_parser.h for enum info)
  Description:
    - Parses configuration file character by character, and equates it with a state. */
NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) {
      break;
    }
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE:
        switch (c) {
          case '{':
            *value = c;
            return TOKEN_TYPE_START_BLOCK;
          case '}':
            *value = c;
            return TOKEN_TYPE_END_BLOCK;
          case '#':
            *value = c;
            state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
            continue;
          case '"':
            *value = c;
            state = TOKEN_STATE_DOUBLE_QUOTE;
            continue;
          case '\'':
            *value = c;
            state = TOKEN_STATE_SINGLE_QUOTE;
            continue;
          case ';':
            *value = c;
            return TOKEN_TYPE_STATEMENT_END;
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
            *value += c;
            state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
            continue;
        }
      case TOKEN_STATE_SINGLE_QUOTE:
        *value += c;
        if (c == '\'') {
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE:
        *value += c;
        if (c == '"') {
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_TOKEN_TYPE_COMMENT:
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        *value += c;
        continue;
      case TOKEN_STATE_TOKEN_TYPE_NORMAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
            c == ';' || c == '{' || c == '}') {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        }
        *value += c;
        continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

/* bool IsValidPortNum(std::string port_token)
  Parameter(s):
    - port_token: Token which contains port value (found from parsing)
  Returns:
    - bool which indicates if port_token is a valid port number
  Description:
    - Helper function that checks to see if given token is a valid port number.  */

bool IsValidPortNum(std::string port_token) {
  try {
    size_t pos = port_token.find(":");
    // handle the case where we are given the IP_Address:Port_number format
    if (pos != std::string::npos) {
      // found a colon
      port_token = port_token.substr(pos+1);
    }
    if (std::stoi(port_token) < 0) {
      return false;
    }
    if (std::stoi(port_token) >= 0 && std::stoi(port_token) <= MAX_PORT) {
      return true;
    }
    return false;
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
  }
}

/* bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config)
  Parameter(s):
    - config_file: Input stream, coming from the configuration file.
    - config: Parsed representation of configuration file (see config_parser.h). Stores the
    resulting parsed information.
  Returns:
    - bool which indicates whether the config file syntax was correct and parse was successful.
  Description:
    - Parses configuration file and catches illogical syntax, stores location path and
    more configuration values into the config object.  */
bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  std::stack<int> bracket_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  bool save_port_val = false;
  bool seen_location = false;
  bool save_root_val = false;
  bool expect_handler_type = false;
  bool expect_static_root = false;
  bool expect_proxy_host = false;
  bool save_proxy_host_val =  false;
  bool expect_proxy_port = false;
  bool save_proxy_port_val = false;
  bool expect_redirect_host = false;
  bool save_redirect_host_val = false;
  // Blog information
  bool expect_blog_info = false;
  bool save_blog_ip_val = false;
  bool save_blog_port = false;
  bool save_blog_username = false;
  bool save_blog_password = false;

  std::string port_token = "port";
  std::string location_token = "location";
  std::string root_token = "root";
  std::string static_token = "StaticHandler";
  std::string echo_token = "EchoHandler";
  std::string status_token = "StatusHandler";
  std::string host_token = "host";
  std::string proxy_token = "ProxyHandler";
  std::string redirect_token = "RedirectHandler";
  std::string health_token = "HealthHandler";
  std::string upload_form_token = "UploadFormHandler";
  std::string blog_token = "BlogHandler";
  std::string username_token = "username";
  std::string password_token = "password";
  std::string location;
  std::string store_proxy_location_token;
  std::string store_redirect_location_token;
  std::string store_blog_ip_location_token;
  std::string store_blog_port_token;
  std::string store_blog_username_token;
  std::string store_blog_password_token;
  std::string store_blog_upload_token;
  std::unordered_set<std::string> seen_handlers;

  BOOST_LOG_TRIVIAL(info) << "Parsing configuration file...";

  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);
    if (save_port_val) {
      SetConfigPortNumberFromToken(token, config);
      BOOST_LOG_TRIVIAL(info) << "Port number: " << token;
      save_port_val = false;
    }
    if (save_proxy_port_val) {
      if (IsValidPortNum(token)) {
        BOOST_LOG_TRIVIAL(info) << "Proxy servlet client location: " << location;
        BOOST_LOG_TRIVIAL(info) << "Proxy servlet server location: " << store_proxy_location_token;
        BOOST_LOG_TRIVIAL(info) << "Proxy servlet server port number: " << token;
        config->proxy_locations_[location] = {store_proxy_location_token, stoi(token)};
      } else {
        std::cerr << "Please provide a proxy server port number\n";
        BOOST_LOG_TRIVIAL(error) << "Port number "
        << std::stoi(port_token) << " is invalid.";
      }
      save_proxy_port_val = false;
      location = "";
    }
    if (save_root_val) {
      // remove the quotes
      token = token.substr(1, token.size()-2);
      BOOST_LOG_TRIVIAL(info) << "Static servlet client location: " << location;
      BOOST_LOG_TRIVIAL(info) << "Static servlet server location: " << token;
      config->static_locations_[location] = token;
      expect_handler_type = false;
      save_root_val = false;
      expect_static_root = false;
      seen_location = false;
      location = "";
    }
    if (save_proxy_host_val) {
      store_proxy_location_token = token.substr(1, token.size() - 2);
      expect_proxy_port = true;
      expect_handler_type = false;
      expect_proxy_host = false;
      save_proxy_host_val = false;
      seen_location = false;
    }
    if (save_redirect_host_val) {
      // remove the quotes
      store_redirect_location_token = token.substr(1, token.size() - 2);
      BOOST_LOG_TRIVIAL(info) << "Proxy servlet client location: " << location;
      BOOST_LOG_TRIVIAL(info) << "Proxy servlet server location: " << store_proxy_location_token;
      config->redirect_locations_[location] = store_redirect_location_token;
      expect_redirect_host = false;
      expect_handler_type = false;
      expect_proxy_port = false;
      save_redirect_host_val = false;
      seen_location = false;
    }
    if (save_blog_ip_val) {
      // remove the quotes
      store_blog_ip_location_token = token.substr(1, token.size() - 2);
      BOOST_LOG_TRIVIAL(info) << "Blog servlet client location: " << location;
      BOOST_LOG_TRIVIAL(info) << "Blog database IP location: " << store_blog_ip_location_token;
      config->blog_ips_[location] = store_blog_ip_location_token;
      save_blog_ip_val = false;
    }
    if (save_blog_port) {
      // remove the quotes
      store_blog_port_token = token.substr(1, token.size() - 2);
      BOOST_LOG_TRIVIAL(info) << "Blog servlet client location: " << location;
      BOOST_LOG_TRIVIAL(info) << "Blog database port: " << store_blog_port_token;
      config->blog_ports_[location] = store_blog_port_token;
      save_blog_port = false;
    }
    if (save_blog_username) {
      // remove the quotes
      store_blog_username_token = token.substr(1, token.size() - 2);
      BOOST_LOG_TRIVIAL(info) << "Blog servlet client location: " << location;
      BOOST_LOG_TRIVIAL(info) << "Blog database username: " << store_blog_username_token;
      config->blog_usernames_[location] = store_blog_username_token;
      save_blog_username = false;
    }
    if (save_blog_password) {
      // remove the quotes
      store_blog_password_token = token.substr(1, token.size() - 2);
      BOOST_LOG_TRIVIAL(info) << "Blog servlet client location: " << location;
      BOOST_LOG_TRIVIAL(info) << "Blog database password: " << store_blog_password_token;
      config->blog_passwords_[location] = store_blog_password_token;
      save_blog_password = false;
    }
    if (expect_handler_type) {
      if (seen_handlers.find(token) == seen_handlers.end()) {
        config->handler_types_.push_back(token);
      }
      seen_handlers.insert(token);
      if (token == static_token) {
        expect_static_root = true;
      } else if (token == redirect_token) {
        expect_redirect_host = true;
      } else if (token == proxy_token) {
        expect_proxy_host = true;
      } else if (token == blog_token) {
        expect_blog_info = true;
      } else if (token == echo_token) {
        BOOST_LOG_TRIVIAL(info) << "Echo location: " << location;
        config->echo_locations_.insert(location);
        location = "";
        seen_location = false;
        expect_handler_type = false;
      } else if (token == status_token) {
        BOOST_LOG_TRIVIAL(info) << "Status location: " << location;
        config->status_locations_.insert(location);
        location = "";
        seen_location = false;
        expect_handler_type = false;
      } else if (token == health_token) {
        BOOST_LOG_TRIVIAL(info) << "Health location: " << location;
        config->health_locations_.insert(location);
        location = "";
        seen_location = false;
        expect_handler_type = false;
      } else if (token == upload_form_token) {
        BOOST_LOG_TRIVIAL(info) << "Upload form location: " << location;
        config->upload_form_locations_.insert(location);
        location = "";
        seen_location = false;
        expect_handler_type = false;
      }
    }
    if (seen_location) {
      location = token.substr(1, token.size()-2);
      expect_handler_type = true;
      seen_location = false;
    }

    if (token == port_token && !expect_proxy_port && !expect_blog_info) {
      save_port_val = true;
    } else if (token == location_token) {
      seen_location = true;
    } else if (expect_static_root && token == root_token) {
      save_root_val = true;
    } else if (expect_proxy_host && token == host_token) {
      save_proxy_host_val = true;
    } else if (expect_proxy_port && token == port_token) {
      save_proxy_port_val = true;
    } else if (expect_redirect_host && token == host_token) {
      save_redirect_host_val = true;
    } else if (expect_blog_info && token == host_token) {
      save_blog_ip_val = true;
    } else if (expect_blog_info && token == port_token) {
      save_blog_port = true;
    } else if (expect_blog_info && token == password_token) {
      save_blog_password = true;
    } else if (expect_blog_info && token == username_token) {
      save_blog_username = true;
    }

    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }

    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }

    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL) {
        if (last_token_type != TOKEN_TYPE_NORMAL) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      } else {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
      NginxConfig* const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
      bracket_stack.push(TOKEN_TYPE_START_BLOCK);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      // Check if '}' does not come after a ';', '}', or '{' which are all the valid preceding chars
      //  or if there is no matching '{'
      if (( (last_token_type != TOKEN_TYPE_STATEMENT_END) &&
            (last_token_type != TOKEN_TYPE_END_BLOCK) &&
            (last_token_type != TOKEN_TYPE_START_BLOCK) )
            || bracket_stack.empty()) {
        // Error.
        break;
      }
      // set our entire state to false, starting a new block
      expect_blog_info = false;
      save_blog_ip_val = false;
      save_blog_port = false;
      save_blog_username = false;
      save_blog_password = false;
      save_port_val = false;
      seen_location = false;
      save_root_val = false;
      expect_handler_type = false;
      expect_static_root = false;
      expect_proxy_host = false;
      save_proxy_host_val =  false;
      expect_proxy_port = false;
      save_proxy_port_val = false;
      expect_redirect_host = false;
      save_redirect_host_val = false;
      bracket_stack.pop();
      config_stack.pop();
    } else if (token_type == TOKEN_TYPE_EOF) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START) {
        // Error.
        break;
      }
      BOOST_LOG_TRIVIAL(info) << "Parsed configuration file successfully.";
      return bracket_stack.empty();
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  BOOST_LOG_TRIVIAL(error) << "Bad transition from "
  << TokenTypeAsString(last_token_type) << " to " <<
  TokenTypeAsString(token_type);
  return false;
}

/* bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config)
  Parameter(s):
    - file_name: Path to configuration file.
    - config: Parsed representation of configuration file (see config_parser.h). Stores the
    resulting parsed information.
  Returns:
    - bool which indicates whether the config file syntax was correct and parse was successful.
  Description:
    - Checks to see if the configuration file is able to be opened. If so, passes to
    overloaded Parse function (above) which does the actual logical parsing.  */
bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open config file: " << file_name;
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();

  return return_value;
}

/* void NginxConfigParser::SetConfigPortNumberFromToken(std::string port_token, NginxConfig* config)
  Parameter(s):
    - port_token: Token which contains port value (found from parsing)
    - config: Parsed representation of configuration file (see config_parser.h). Stores the
    resulting parsed information.
  Returns:
    - N/A
  Description:
    - Checks to see if the port value provided in config is valid.  */
void NginxConfigParser::SetConfigPortNumberFromToken(std::string port_token, NginxConfig* config) {
  try {
    size_t pos = port_token.find(":");
    // handle the case where we are given the IP_Address:Port_number format
    if (pos != std::string::npos) {
      // found a colon
      port_token = port_token.substr(pos+1);
    }
    if (std::stoi(port_token) < 0) {
      std::cerr << "Please provide a valid port number\n";
      BOOST_LOG_TRIVIAL(error) << "Port number "
      << std::stoi(port_token) << " is invalid.";
    }
    if (std::stoi(port_token) >= 0 && std::stoi(port_token) <= MAX_PORT) {
      // we are given a valid port number, set our config object's port number member
      config->port_number = std::stoi(port_token);
    }
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
  }
}
