/* request.h
Header file for creating HTTP requests.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>

class Request {
    public:
        // The HTML method (GET, PUT, POST, etc)
        enum MethodEnum {
            GET,
            HEAD,
            POST,
            PUT,
            DELETE,
            CONNECT,
            OPTIONS,
            TRACE
        };
        MethodEnum method_;

        // The path of the request
        std::string uri_;

        // The HTTP version string as given in the request line, e.g. "HTTP/1.1"
        std::string version_;

        // A map of header, for convenient lookup ("Content-Type", "Cookie", etc)
        std::map<std::string, std::string> headers_;

        // The content of the request
        std::string body_;
};

#endif // HTTP_REQUEST_HPP
