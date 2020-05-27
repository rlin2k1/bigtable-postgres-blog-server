/* request_dispatcher.cc
Description:
    Initializes and dispatches requests based on
    URI from configuration information.

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
#include "proxy_request_handler.h"
#include "redirect_request_handler.h"
#include "health_request_handler.h"

/* request_dispatcher Constructor
Parameter(s):
    - config: parsed representation of configuration file (see config_parser.h)
Description:
    - Calls function which initializes handlers corresponding to those specified in the config. */
request_dispatcher::request_dispatcher(const NginxConfig& config): config_(config) {
    create_handler_mapping(); // Initializes the needed request handlers
}

/* void request_dispatcher::create_handler_mapping()
Parameter(s):
    - N/A
Returns:
    - N/A
Description:
    - References information from config member variable to initialize various handler types. */
void request_dispatcher::create_handler_mapping() {
    std::vector<std::string> handler_types = config_.handler_types_;

    // Iterates through each handler type available in the config. Should be UNIQUE
    for (std::vector<std::string>::const_iterator i = handler_types.begin(); i != handler_types.end(); ++i) {
        if (*i == "EchoHandler") {
            std::unordered_set<std::string> echo_locations = config_.echo_locations_;
            for (std::unordered_set<std::string>::iterator itr = echo_locations.begin(); itr != echo_locations.end(); ++itr) {
                request_handler* echo_handler = echo_request_handler::Init(*itr, config_);

                dispatcher[*itr] = echo_handler;  // Set echo uri path mapping to echo handler
            }
  	    }
  	    else if (*i == "StaticHandler") {
            std::unordered_map<std::string, std::string> static_locations = config_.static_locations_;

            for (std::unordered_map<std::string, std::string>::iterator itr = static_locations.begin(); itr != static_locations.end(); ++itr) {
                request_handler* static_handler = static_request_handler::Init(itr->first, config_);

                dispatcher[itr->first] = static_handler;  // Set static uri path mapping to static handler
            }
  	    }
        else if (*i == "StatusHandler") {
            std::unordered_set<std::string> status_locations = config_.status_locations_;
            for (std::unordered_set<std::string>::iterator itr = status_locations.begin(); itr != status_locations.end(); ++itr) {
                request_handler* status_handler = status_request_handler::Init(*itr, config_);

                dispatcher[*itr] = status_handler;  // Set status uri path mapping to echo handler
                status_handler_enabled = true;
            }
        } else if (*i == "ProxyHandler") {
          std::unordered_map<std::string, std::pair<std::string, int>> proxy_locations = config_.proxy_locations_;
          for (std::unordered_map<std::string, std::pair<std::string, int>>::iterator itr = proxy_locations.begin(); itr != proxy_locations.end(); ++itr) {
            request_handler* proxy_handler = proxy_request_handler::Init(itr->first, config_);

            dispatcher[itr->first] = proxy_handler;  // Set proxy uri path mapping to proxy handler
          }
        } else if (*i == "RedirectHandler") {
          auto& redirect_locations = config_.redirect_locations_;
          for (auto itr = redirect_locations.begin(); itr != redirect_locations.end(); ++itr) {
            request_handler* redirect_handler = redirect_request_handler::Init(itr->first, config_);

            dispatcher[itr->first] = redirect_handler;  // Set redirect uri path mapping to redirect handler
          } 
        } else if (*i == "HealthHandler") {
          std::unordered_set<std::string> health_locations = config_.health_locations_;

          for (std::unordered_set<std::string>::iterator itr = health_locations.begin(); itr != health_locations.end(); ++itr) {
              request_handler* health_handler = health_request_handler::Init(*itr, config_);
              dispatcher[*itr] = health_handler;  // Set health uri path mapping to health handler
          }
        }
        // ******************************** TEMPLATE FOR NEW HANDLER REGISTRATIONS *******************************
        // else if (*i == "NEWHandler") {  // TODO (newteam): Add a new handler. Your handler may not need an unordered_map to track locations
    	//     request_handler* NEW_handler = NEW_request_handler::Init(config_);

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

/* request_handler* request_dispatcher::get_handler(std::string uri)
Parameter(s):
    - uri: URI given for a request handler in the config.
Returns:
    - Base class pointer to corresponding handler type.
Description:
    - Returns base class pointer with handler respective to URI provided. */
request_handler* request_dispatcher::get_handler(std::string uri) {
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
    } else if (config_.redirect_locations_.find(uri) != config_.redirect_locations_.end()) {
        return dispatcher[uri];
    } else if (config_.health_locations_.find(uri) != config_.health_locations_.end()) {
        return dispatcher[uri];
    }

    std::string uri_prefix;
    bool found = false;
    for (const auto& pair : config_.proxy_locations_) {
        uri_prefix = uri.substr(0, pair.first.length());
        if (uri_prefix == pair.first) {
            found = true;
            break;
        }
    }

    if (found) {
        return dispatcher[uri_prefix];
    }

    return error_handler_;
    // ******* TEMPLATE FOR DISPATCHING NEW HANDLERS ********
    // else if (your condition here) {
    //     return dispatcher[your path]
    // }
    // ******************************************************
}

/* request_handler* request_dispatcher::get_handler(std::string uri)
Parameter(s):
    - N/A
Returns:
    - Pointer specific to status_request_handler type.
Description:
    - Returns a pointer to status handler. Used when session.cc needs to record
    requests that the server has received. */
status_request_handler* request_dispatcher::get_status_handler() {
    request_handler* status_handler_ptr = dispatcher["/status"];
    status_request_handler* casted_ptr = dynamic_cast<status_request_handler*>(status_handler_ptr);
    return casted_ptr;
}

/* std::string request_dispatcher::longest_prefix_match(std::string uri)
Parameter(s):
    - uri: String that stores URI for static_request_handler.
Returns:
    - Trimmed URI string to use to retrieve the correct static handler.
Description:
    - Helper function to retrieve the URI for static handler. */
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
