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

#include "request.h"
#include "reply.h"
#include "static_request_handler.h"

namespace http {
namespace server {
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
      // TODO(mkgee): add a default handle bad request
      // that can handle various bad requests
      rep.status = reply::not_found;
      rep.content = http::server::stock_replies::not_found;
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

  void static_request_handler::handle_request(const request& req, reply& rep,  const char *data) {
    // Fill out the reply to be sent to the client.
    std::string file_name = config_->static_locations_[target_dir_] + partial_uri_;
    std::ifstream send_file;
    send_file.open(file_name.c_str());
    if (!send_file.good()) {
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
