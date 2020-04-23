#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

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

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  std::stack<int> bracket_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  bool save_port_val = false;
  bool save_location_val = false;
  bool save_server_location_val = false;
  bool save_client_location_val = false;
  bool seen_servlet = false;
  bool seen_static = false;
  bool seen_echo = false;
  bool save_root_val = false;

  std::string port_token = "listen";
  std::string location_token = "location";
  std::string server_location_token = "server_location";
  std::string client_location_token = "client_location";
  std::string servlet_token = "servlet";
  std::string root_token = "root";
  std::string static_token = "static";
  std::string echo_token = "echo";

  std::vector<std::string> client_locations;
  std::vector<std::string> server_locations;


  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);

    if (save_port_val) {
      SetConfigPortNumberFromToken(token, config);
      save_port_val = false;
    }
    if (save_root_val) {
      config->root_path_ = token;
      save_root_val = false;
    }
    if (seen_servlet && seen_static && save_server_location_val) {
      server_locations.push_back(token);
      save_server_location_val = false;
    } else if (seen_servlet && seen_static && save_client_location_val) {
      client_locations.push_back(token);
      save_client_location_val = false;
    }
    if (server_locations.size() > 0 && client_locations.size() > 0) {
      config->static_locations_[client_locations[0]] = server_locations[0];
      server_locations.erase(server_locations.begin());
      client_locations.erase(client_locations.begin());
      seen_servlet = false;
      seen_static = false;
    }
    if (seen_servlet && seen_echo && save_location_val) {
      config->echo_location_ = token;
      seen_servlet = false;
      seen_static = false;
      save_location_val = false;
    }

    if (token == port_token) {
      save_port_val = true;
    } else if (token == location_token) {
      save_location_val = true;
    } else if (token == servlet_token) {
      seen_servlet = true;
    } else if (token == root_token) {
      save_root_val = true;
    } else if (token == static_token) {
      seen_static = true;
    } else if (token == echo_token) {
      seen_echo = true;
    } else if (token == server_location_token) {
      save_server_location_val = true;
    } else if (token == client_location_token) {
      save_client_location_val = true;
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
      return bracket_stack.empty();
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  printf ("Bad transition from %s to %s\n",
          TokenTypeAsString(last_token_type),
          TokenTypeAsString(token_type));
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf ("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

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
    }
    if (std::stoi(port_token) >= 0 && std::stoi(port_token) <= MAX_PORT) {
      // we are given a valid port number, set our config object's port number member
      config->port_number = std::stoi(port_token);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
