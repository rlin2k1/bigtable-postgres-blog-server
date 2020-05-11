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

#define DIR_INDEX 1

request_dispatcher::request_dispatcher(NginxConfig* config): config_(config) {
    create_handler_mapping(); // Initializes the needed request handlers
}

void request_dispatcher::create_handler_mapping() {
    std::vector<std::string> handler_types = config_->handler_types_;

    // Iterates through each handler type available in the config. Should be UNIQUE
    for(std::vector<std::string>::const_iterator i = handler_types.begin(); i != handler_types.end(); ++i) {
        if (*i == "EchoHandler") {
            http::server::request_handler* echo_handler = http::server::echo_request_handler::Init(config_);

            std::unordered_set<std::string> echo_locations = config_->echo_locations_;
            for (std::unordered_set<std::string>::iterator itr = echo_locations.begin(); itr != echo_locations.end(); ++itr) {
                dispatcher[*itr] = echo_handler; // Set echo uri path mapping to echo handler
            }

  	    }
  	    else if (*i == "StaticHandler") {
    	    http::server::request_handler* static_handler = http::server::static_request_handler::Init(config_);

            std::unordered_map<std::string, std::string> static_locations = config_->static_locations_;

            for (std::unordered_map<std::string, std::string>::iterator itr = static_locations.begin(); itr != static_locations.end(); ++itr) {
                dispatcher[itr->first] = static_handler; // Set static uri path mapping to static handler
            }
  	    }
        // else if (*i == "NEWHandler") { // TODO (NEWTEAM): Add a new handler. Your handler may not need an unordered_map to track locations
    	//     http::server::request_handler* NEW_handler = http::server::NEW_request_handler::Init(config_);

        //     std::unordered_map<std::string, std::string> static_locations = config_->NEW_locations_;???

        //     for (std::unordered_map<std::string, std::string>::iterator itr = NEW_locations.begin(); itr != NEW_locations.end(); ++itr) {
        //         dispatcher[itr->first] = NEW_handler;
        //     }
  	    // }

        // If handler does not exist, do not add it to the dispatcher map
        // Default Bad Request Handler will handle! 
        else {
            http::server::request_handler* error_404_request_handler = http::server::error_404_request_handler::Init(config_);
            dispatcher["404"] = error_404_request_handler; // Set echo uri path mapping to echo handler
        }
    }
}

http::server::request_handler* request_dispatcher::get_handler(std::string uri) {
    // Find the root directory and target file from the client's request uri
    // TODO (Kubilay): Longest prefix match
    std::vector<std::string> path_elements;
    size_t space_index = 0;
    while (true) {
        space_index = uri.find("%20", space_index);
        if (space_index == std::string::npos) {
            break;
        }
        uri.replace(space_index, 3, " ");
    }
    boost::split(path_elements, uri, boost::is_any_of("/"));
    std::string target_dir = "/" + path_elements[DIR_INDEX];

    // Call the corresponding handler to handle the request
    if (config_->echo_locations_.find(uri) != config_->echo_locations_.end()) {
        return dispatcher[uri];
    } else if (config_->static_locations_.find(target_dir) != config_->static_locations_.end()) {
        return dispatcher[target_dir];
    }  else {
        return dispatcher["404"];
    }
}
