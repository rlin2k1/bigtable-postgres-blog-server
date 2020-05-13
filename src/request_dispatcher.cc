/* request_dispatcher.cc
Dispatches requests based on URI.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 8th, 2020
*/

#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "request_dispatcher.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"
#include "error_404_request_handler.h"
#include "status_request_handler.h"

request_dispatcher::request_dispatcher(const NginxConfig& config): config_(config) {
    create_handler_mapping(); // Initializes the needed request handlers
}

void request_dispatcher::create_handler_mapping() {
    std::vector<std::string> handler_types = config_.handler_types_;

    // Iterates through each handler type available in the config. Should be UNIQUE
    for (std::vector<std::string>::const_iterator i = handler_types.begin(); i != handler_types.end(); ++i) {
        if (*i == "EchoHandler") {
            std::unordered_set<std::string> echo_locations = config_.echo_locations_;
            for (std::unordered_set<std::string>::iterator itr = echo_locations.begin(); itr != echo_locations.end(); ++itr) {
                http::server::request_handler* echo_handler = http::server::echo_request_handler::Init(*itr, config_);

                dispatcher[*itr] = echo_handler;  // Set echo uri path mapping to echo handler
            }

  	    }
  	    else if (*i == "StaticHandler") {
            std::unordered_map<std::string, std::string> static_locations = config_.static_locations_;

            for (std::unordered_map<std::string, std::string>::iterator itr = static_locations.begin(); itr != static_locations.end(); ++itr) {
                http::server::request_handler* static_handler = http::server::static_request_handler::Init(itr->first, config_);

                dispatcher[itr->first] = static_handler;  // Set static uri path mapping to static handler
            }
  	    }
        else if (*i == "StatusHandler") {
            std::unordered_set<std::string> status_locations = config_.status_locations_;
            for (std::unordered_set<std::string>::iterator itr = status_locations.begin(); itr != status_locations.end(); ++itr) {
                http::server::request_handler* status_handler = http::server::status_request_handler::Init(*itr, config_);

                dispatcher[*itr] = status_handler;  // Set status uri path mapping to echo handler
                status_handler_enabled = true;
            }
  	    }
        // ******************************** TEMPLATE FOR NEW HANDLER REGISTRATIONS *******************************
        // else if (*i == "NEWHandler") {  // TODO (newteam): Add a new handler. Your handler may not need an unordered_map to track locations
    	//     http::server::request_handler* NEW_handler = http::server::NEW_request_handler::Init(config_);

        //     std::unordered_map<std::string, std::string> static_locations = config_.NEW_locations_;???

        //     for (std::unordered_map<std::string, std::string>::iterator itr = NEW_locations.begin(); itr != NEW_locations.end(); ++itr) {
        //         dispatcher[itr->first] = NEW_handler;
        //     }
  	    // }
        // *********************************************************************************************************
        // If handler does not exist, do not add it to the dispatcher map
        // Default Bad Request Handler will handle!
    }
}

http::server::request_handler* request_dispatcher::get_handler(std::string uri) {
    // Find the root directory and target file from the client's request uri
    size_t space_index = 0;
    while (true) {
        space_index = uri.find("%20", space_index);
        if (space_index == std::string::npos) {
            break;
        }
        uri.replace(space_index, 3, " ");
    }

    std::string static_path = longest_prefix_match(uri);

    // Call the corresponding handler to handle the request
    if (config_.echo_locations_.find(uri) != config_.echo_locations_.end()) {
        return dispatcher[uri];
    } else if (static_path != "") {
        return dispatcher[static_path];
    } else if (config_.status_locations_.find(uri) != config_.status_locations_.end()) {
        return dispatcher[uri];
    } else {
        return error_handler_;
    }
    // ******* TEMPLATE FOR DISPATCHING NEW HANDLERS ********
    // else if (your condition here) {
    //     return dispatcher[your path]
    // }
    // ******************************************************
}

http::server::status_request_handler* request_dispatcher::get_status_handler() {
    http::server::request_handler* status_handler_ptr = dispatcher["/status"];
    http::server::status_request_handler* casted_ptr = dynamic_cast<http::server::status_request_handler*>(status_handler_ptr);
    return casted_ptr;
}

std::string request_dispatcher::longest_prefix_match(std::string uri) {
    size_t slash_pos = 0;
    std::string trimmed_uri = uri;
    while (true) {
        if (config_.static_locations_.find(trimmed_uri) != config_.static_locations_.end()) {
            return trimmed_uri;
        }
        slash_pos = trimmed_uri.find_last_of("/");
        if (slash_pos == std::string::npos) {
            break;
        }
        trimmed_uri = trimmed_uri.substr(0, slash_pos);
    }
    return "";
}
