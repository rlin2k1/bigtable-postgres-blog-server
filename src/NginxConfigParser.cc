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

  std::string port_token = "port";
  std::string location_token = "location";
  std::string root_token = "root";
  std::string static_token = "StaticHandler";
  std::string echo_token = "EchoHandler";
  std::string status_token = "StatusHandler";

  std::string location;
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
    if (save_root_val) {
      // remove the quotes
      location = location.substr(1, location.size()-2);
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
    if (expect_handler_type) {
      if (seen_handlers.find(token) == seen_handlers.end()) {
        config->handler_types_.push_back(token);
      }
      seen_handlers.insert(token);
      if (token == static_token) {
        expect_static_root = true;
      } else if (token == echo_token) {
        // remove the quotes
        location = location.substr(1, location.size()-2);
        BOOST_LOG_TRIVIAL(info) << "Echo location: " << location;
        config->echo_locations_.insert(location);
        location = "";
        seen_location = false;
        expect_handler_type = false;
      } else if (token == status_token) {
        // remove the quotes
        location = location.substr(1, location.size()-2);
        BOOST_LOG_TRIVIAL(info) << "Status location: " << location;
        config->status_locations_.insert(location);
        location = "";
        seen_location = false;
        expect_handler_type = false;
      }
    }
    if (seen_location) {
      location = token;
      expect_handler_type = true;
      seen_location = false;
    }

    if (token == port_token) {
      save_port_val = true;
    } else if (token == location_token) {
      seen_location = true;
    } else if (expect_static_root && token == root_token) {
      save_root_val = true;
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
