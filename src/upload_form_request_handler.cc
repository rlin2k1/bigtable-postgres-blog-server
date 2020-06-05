/* upload_form_request_handler.cc
Description:
    Request handler for request to get the html form to submit blog entries

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 4th, 2020
*/

#include <fstream>
#include <sstream>
#include <string>
#include <boost/log/trivial.hpp>

#include "upload_form_request_handler.h"
#include "response_helper_library.h"

/* upload_form_request_handler* Init(const std::string& location_path, const NginxConfig& config)
Parameter(s):
    - location_path: path provided in config file which corresponds to handler
    - config: parsed representation of configuration file (see config_parser.h)
Returns:
    - Pointer to a upload_form_request_handler object.
Description:
    - Uses values provided in the function's parameter to initialize a upload_form_request_handler object */
upload_form_request_handler* upload_form_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    // NginxConfig is not used for the upload form.
    upload_form_request_handler* ufrh = new upload_form_request_handler();
    // TODO (kubilayagi): update styling if there's time later on
    ufrh->form_html_ = "<!DOCTYPE html>\n\
<html>\n\
    <head>\n\
        <meta charset='utf-8'>\n\
        <title>Blog Upload Form</title>\n\
    </head>\n\
    <body>\n\
        <form action='/blog' method='POST'>\n\
        <div>\n\
            <label for='submissiontitle'>Title</label>\n\
            <input name='submissiontitle' id='submissiontitle' placeholder='Blog Title'>\n\
        </div>\n\
        <div>\n\
            <label for='submissionbody'>Body</label>\n\
            <textarea name='submissionbody' id='submissionbody' placeholder='Blog Body Text'></textarea>\n\
        </div>\n\
        <div>\n\
            <button>Submit</button>\n\
        </div>\n\
        </form>\n\
    </body>\n\
</html>\n";
    return ufrh;
}

/*  Response upload_form_request_handler::handle_request(const request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - Response object (see response.h)
Description:
    - Response object is generated and returned using preformatted and populated html form
    for users. */
Response upload_form_request_handler::handle_request(const Request& request) {
    BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]Request_Handler: upload_form" ;
    BOOST_LOG_TRIVIAL(info) << "Sending back blog upload form.";
    Response response;

    // Fill out the Response to be sent to the client.
    response.code_ = Response::ok;
    response.body_ = form_html_;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/html";

    return response;
}
