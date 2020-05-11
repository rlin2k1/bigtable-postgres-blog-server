/* static_request_handler.cc
Request handler to serve static asset responses.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#include <fstream>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "request.h"
#include "reply.h"
#include "static_request_handler.h"

#define DIR_INDEX 1

namespace http {
namespace server {
  static_request_handler::static_request_handler(NginxConfig* config) : config_(config){}

  static_request_handler* static_request_handler::Init(NginxConfig* config) {
        return new static_request_handler(config);
    }

  std::unordered_map<std::string, std::string> mappings(
  {
      { "gif", "image/gif" },
      { "htm", "text/html" },
      { "html", "text/html" },
      { "jpg", "image/jpeg" },
      { "jpeg", "image/jpeg" },
      { "png", "image/png" },
      { "zip", "application/zip" },
      { "txt", "text/plain" },
      { "pdf", "application/pdf" }
  });

  // Return a Not found reply if there are no echo or
  // static uris that can be handled
  void static_request_handler::default_handle_bad_request(reply& rep) {
      // TODO(Jane): add a default handle bad request
      // that can handle various bad requests
      // TODO(Jane): Is there a way to leave this object and instantiate 
      // a new bad handler object in request handler? 
      BOOST_LOG_TRIVIAL(info) << "Unable to open file - bad request.";
      rep.status = reply::bad_request;
      rep.content = http::server::stock_replies::bad_request;
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/html";
  }

// Get mapping of mime type
  std::string static_request_handler::get_mime_type(std::string file_name) {
    size_t last_dot_index = file_name.find_last_of(".");
    if (last_dot_index != std::string::npos) {
      std::string file_extension = file_name.substr(last_dot_index + 1);
      if (mappings.find(file_extension) != mappings.end()) {
        return mappings[file_extension];
      }
    }
    return "text/plain";
  }

  void static_request_handler::handle_request(request& req, reply& rep,  const char *data) {
    // Find the root directory and target file from the client's request uri
    std::vector<std::string> path_elements;
    size_t space_index = 0;
    while (true) {
        space_index = req.uri.find("%20", space_index);
        if (space_index == std::string::npos) {
            break;
        }
        req.uri.replace(space_index, 3, " ");
    }

    boost::split(path_elements, req.uri, boost::is_any_of("/"));
    std::string target_dir = "/" + path_elements[DIR_INDEX];
    std::string target_file = path_elements[path_elements.size() - 1];

    // Rebuild the uri without the root folder, which is added in the request handler
    std::string partial_uri;
    for (int i = DIR_INDEX + 1; i < path_elements.size(); i++) {
        partial_uri = partial_uri + "/" + path_elements[i];
    }

    target_dir_ = target_dir;
    target_file_ = target_file;
    partial_uri_ = partial_uri;

    BOOST_LOG_TRIVIAL(info) << "Currently serving static requests on path: " << target_dir << partial_uri;

    //--------------------------------------------------------------------------
    // Fill out the reply to be sent to the client.
    std::string file_name = config_->static_locations_[target_dir_] + partial_uri_;
    std::ifstream send_file;
    send_file.open(file_name.c_str());
    if (!send_file.good()) {
      BOOST_LOG_TRIVIAL(error) << "Could not open file at path: " << file_name;
      default_handle_bad_request(rep);
    } else {
      std::vector<char> send_data;
      char c = '\0';
      while (send_file.get(c)) {
        send_data.push_back(c);
      }
      std::string send_data_string(send_data.begin(), send_data.end());
      rep.status = reply::ok;
      rep.content = send_data_string;
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = get_mime_type(target_file_);
    }
  }
}  // namespace server
}  // namespace http
