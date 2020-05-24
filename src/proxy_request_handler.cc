/* proxy_request_handler.cc
Description:
    Request handler to serve reverse proxy responses.

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 20th, 2020
*/

#include <string>
#include <unordered_map>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <libxml2/libxml/HTMLtree.h>
#include <algorithm>

#include "response_parser.h"
#include "response_builder.h"
#include "proxy_request_handler.h"
#include "response_helper_library.h"

#define HTTP_DEFAULT_PORT 80

using namespace boost::asio;
using namespace boost::iostreams;

/* proxy_request_handler* Init(const std::string& location_path, const NginxConfig& config)
Parameter(s):
    - location_path: path provided in config file which corresponds to handler
    - config: parsed representation of configuration file (see config_parser.h)
Returns:
    - Pointer to a proxy_request_handler object.
Description:
    - Uses values provided in the function's parameter to initialize a proxy_request_handler object */
proxy_request_handler* proxy_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    proxy_request_handler* prh = new proxy_request_handler();
    prh -> client_location_path_ = location_path;
    prh -> server_port_num = config.proxy_locations_.at(location_path).second;

    // Extract URL from the host in the config and separate URI into its own
    //  variable
    std::string server_url = config.proxy_locations_.at(location_path).first;
    size_t pos = server_url.find('/');
    if (pos != std::string::npos) {
        prh -> server_url_ = server_url.substr(0, pos);
        prh -> server_location_path_ = server_url.substr(pos);
        if (*(prh -> server_location_path_.rbegin()) == '/') {
            size_t len = prh -> server_location_path_.length();
            prh -> server_location_path_ =
                prh -> server_location_path_.substr(0, len - 1);
        }
    } else {
        prh -> server_url_ = server_url;
    }

    return prh;
}

/* std::string build_request_string(const Request& request)
Parameter(s):
    - request: request to convert to string
Returns:
    - String representation of given request.
Description:
    - Converts given request into a string */
std::string proxy_request_handler::build_request_string(const Request& request) {
    std::string request_string;
    if (request.method_ == Request::MethodEnum::GET) {
        request_string += "GET ";
    } else if (request.method_ == Request::MethodEnum::POST) {
        request_string += "POST ";
    } else if (request.method_ == Request::MethodEnum::DELETE) {
        request_string += "DELETE ";
    } else if (request.method_ == Request::MethodEnum::HEAD) {
        request_string += "HEAD ";
    } else if (request.method_ == Request::MethodEnum::PUT) {
        request_string += "PUT ";
    } else if (request.method_ == Request::MethodEnum::CONNECT) {
        request_string += "CONNECT ";
    } else if (request.method_ == Request::MethodEnum::OPTIONS) {
        request_string += "OPTIONS ";
    } else if (request.method_ == Request::MethodEnum::TRACE) {
        request_string += "TRACE ";
    }

    request_string += request.uri_;
    request_string += " ";
    request_string += request.version_;
    request_string += "\r\n";

    for (const auto& pair : request.headers_) {
        std::string new_headerline = pair.first + ": " + pair.second + "\r\n";
        request_string += new_headerline;
    }
    request_string += "\r\n";

    request_string += request.body_;
    return request_string;
}

/* void handle_property(xmlNode *node, const char *property)
Parameter(s):
    - node: xml node to process
    - property: xml property to alter
Description:
    - Converts the property of node to be a valid path for our proxy server */
void proxy_request_handler::handle_property(xmlNode *node, const char *property) {
    const char *propCString =
        (const char*)xmlGetProp(node, (const xmlChar*)property);
    if (propCString == nullptr) {
        return;
    }

    std::string prop(propCString);

    if (prop.length() > 1 && prop[0] == '#') {
        return;
    }

    if (prop.length() > 2 && prop.substr(0,2) == "//") {
        return;
    }

    if (prop.length() > 4 && prop.substr(0, 4) == "http") {
        return;
    }

    if (prop[0] != '/') {
        prop.insert(0, "/");
    }

    prop.insert(0, client_location_path_);
    xmlSetProp(node, (const xmlChar*)property, (const xmlChar*)prop.data());
}

/* void modify_html_doc(xmlNode *node)
Parameter(s):
    - node: xml root to process
Description:
    - Converts the properties of node to be a valid paths for our proxy server recursively*/
void proxy_request_handler::modify_html_doc(xmlNode *node) {
    if (node == NULL) {
        return;
    }

    xmlNode *curr;
    for (curr = node; curr != nullptr; curr = curr->next) {
        if (curr->type == XML_ELEMENT_NODE) {
            handle_property(curr, "href");
            handle_property(curr, "src");
        }
        modify_html_doc(curr->children);
    }
}

/* bool read_response(ip::tcp::socket& socket, Response &response)
Parameter(s):
    - socket: xml root to process
    - response: returned response from reading the socket
Returns:
    - boolean denoting if there was an error while reading
Description:
    - Reads from socket and parses the HTTP response*/
bool proxy_request_handler::read_response(ip::tcp::socket& socket, Response &response) {
    response_builder builder;
    response_parser parser;
    response_parser::result_type result;

    char data_[max_length];
    bool done_reading = false;
    bool error = false;
    while (!done_reading) {
        boost::system::error_code ec;
        std::size_t bytesRead = socket.read_some(buffer(data_, max_length), ec);

        if (ec) {
            error = true;
            break;
        }

        std::tie(result, std::ignore) = parser.parse(
              builder, data_, data_ + bytesRead);

        if (result == response_parser::good || result == response_parser::bad) {
            done_reading = true;
        }
    }

    if (result == response_parser::bad || error) {
        return false;
    }

    response = builder.build_response();
    return true;
}

/* bool std::string decompress_gzip(std::string compressed)
Parameter(s):
    - compressed: compressed gzip file of HTML document
Returns:
    - string of decompressed HTML document
Description:
    - Decompresses a gzip HTML document */
std::string proxy_request_handler::decompress_gzip(std::string compressed) {
    std::stringstream htmlCompressed(compressed);
    std::stringstream htmlDecompressed;

    filtering_streambuf<input> buff;
    buff.push(gzip_decompressor());
    buff.push(htmlCompressed);
    boost::iostreams::copy(buff, htmlDecompressed);

    return htmlDecompressed.str();
}

/* bool Response handle_html(Response& response)
Parameter(s):
    - response: response from a remote server with HTML
Returns:
    - A response consisting of modified HTML with links replaced correctly
Description:
    - Creates a response object with an HTML body such that the links
       correspond correctly to the proxy server instead of the remote server */
Response proxy_request_handler::handle_html(Response& response) {
    std::string htmlContent = response.body_;

    // If The content encoding is gzip, decompress it
    if (response.headers_.find("Content-Encoding") != response.headers_.end()) {
        std::string contentEncoding = response.headers_["Content-Encoding"];
        if (contentEncoding.find("gzip") != std::string::npos) {
            htmlContent = decompress_gzip(htmlContent);
        }

        // Mark the response as being no encoding since we arent going to
        //  recompress it
        response.headers_["Content-Encoding"] = "identity";
    }

    // Parse the html document
    htmlDocPtr htmlDoc = htmlReadMemory(
        htmlContent.data(),
        htmlContent.length(),
        server_url_.data(),
        NULL,
        XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

    // modify the document so that the paths are corrected
    modify_html_doc(xmlDocGetRootElement(htmlDoc));

    // Dump the HTML into a string
    xmlChar *xmlString;
    int size;
    htmlDocDumpMemory(htmlDoc, &xmlString, &size);
    htmlContent = (char*)xmlString;
    xmlFree(xmlString);
    xmlFree(htmlDoc);

    // Set the body
    response.body_ = htmlContent;
    // Make sure the encoding is set correctly
    response.headers_["Transfer-Encoding"] = "identity";
    // Set the content length to the length of the HTML document
    response.headers_["Content-Length"] = std::to_string(htmlContent.length());
    return response;
}

/* Response get_error_response()
Returns:
    - A response object
Description:
    - Creates an error Response object to be used throughout the handler */
Response proxy_request_handler::get_error_response() {
    Response response;
    response.code_ = Response::bad_gateway;
    response.body_ = stock_responses::bad_gateway;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/html";

    return response;
}

/* bool Response proxy_request(const Request& request, std::string uri, std::string url, int port)
Parameter(s):
    - request: original request to proxy
    - uri: URI to send proxyed request to
    - url: URL to send request to
    - port: port number of remote server
Returns:
    - A valid response for the given request and proxy info
Description:
    - Sends a proxied request to URL and parses response. Modifies HTML if
       needed to maintain proxy */
Response proxy_request_handler::proxy_request(const Request& request, std::string uri, std::string url, int port) {
    Response response;
    std::string portString = std::to_string(port);

    BOOST_LOG_TRIVIAL(info) << "Proxying Request to URL " << url
        << " at port " << port << " with URI " << uri;

    // Set our proxy request
    Request proxyRequest;
    proxyRequest.method_ = request.method_;
    proxyRequest.uri_ = uri;
    proxyRequest.version_ = request.version_;
    proxyRequest.body_ = request.body_;
    proxyRequest.headers_ = request.headers_;
    // Set host url and port correctly
    proxyRequest.headers_["Host"] = url + ":" + portString;
    // We can only handle respoonses with either gzip or no encoding
    //  We must set the HTTP Header to only accept information types our server
    //  can handle
    proxyRequest.headers_["Accept-Encoding"] = "gzip, identity";

    streambuf responseStatusLineBuffer;

    // Connect to the remote server
    io_service ioService;
    ip::tcp::socket socket(ioService);
    ip::tcp::resolver resolver(ioService);
    ip::tcp::resolver::query query(
        url,
        std::to_string(server_port_num));
    connect(socket, resolver.resolve(query));

    // Send them the request
    std::string proxyRequestString = build_request_string(proxyRequest);
    write(socket, buffer(proxyRequestString.data(), proxyRequestString.size()));

    // Try to read and parse the response, if we get an error, return an error
    if (!read_response(socket, response)) {
        return get_error_response();
    }

    // Check if response is a redirect
    if (response.code_ == Response::moved_temporarily
        || response.code_ == Response::moved_permanently) {
        std::string redirectLocation = response.headers_["Location"];
        int newPort = HTTP_DEFAULT_PORT;

        // Check if it is an absolute or relative path
        size_t pos = redirectLocation.find("//");

        // If there is no "//", then it is a relative path on the same server
        if (pos == std::string::npos) {
            // If it is the same server, the Location field is the URI
            uri = redirectLocation;
            // The new URL is the old URL since it is relative
            redirectLocation = url;
            // The new port is the old port as well
            newPort = port;

            return proxy_request(request, uri, redirectLocation, newPort);
        }

        // Remove the starting http:// or https://
        redirectLocation = redirectLocation.substr(pos + 2);
        uri = "/";

        // Get URI at the end
        pos = redirectLocation.find('/');
        if (pos != std::string::npos) {
            uri = redirectLocation.substr(pos);
            redirectLocation = redirectLocation.substr(0, pos);
        }

        // Get port number following the colon
        pos = redirectLocation.find(':');
        if (pos != std::string::npos) {
            newPort = stoi(redirectLocation.substr(pos + 1));
            redirectLocation = redirectLocation.substr(0, pos);
        }

        BOOST_LOG_TRIVIAL(info) << "Redirecting Request to URL " << redirectLocation
            << " at port " << newPort << " with URI " << uri;
        return proxy_request(request, uri, redirectLocation, newPort);
    }

    // If the content type is HTML, modify the HTML to work as a proxy
    //  effectively
    std::string contentType = response.headers_["Content-Type"];
    if (contentType.find("text/html") != std::string::npos) {
        response = handle_html(response);
    }

    return response;
}

/*  Response proxy_request_handler::handle_request(const request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - Response object (see response.h)
Description:
    - Handler uses request URI to get full proxy destination, issue a request to that destination,
    and return the response. Also handles HTTP 302 Redirect. */
Response proxy_request_handler::handle_request(const Request& request) {
    // Get the URI, the new URI will be the URI given here + the server path
    std::string uri = request.uri_.substr(client_location_path_.length());
    uri = server_location_path_ + uri;
    // Make sure to have a URI if both uri and server_location_path_ are empty
    if (uri.empty()) {
        uri = "/";
    }

    return proxy_request(request, uri, server_url_, server_port_num);
}
