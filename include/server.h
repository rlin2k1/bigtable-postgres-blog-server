/* server.h
Header file for server class which starts the io service.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 8th, 2020
*/

#include <boost/asio.hpp>

#include "config_parser.h"
#include "request_dispatcher.h"

class session;

using boost::asio::ip::tcp;

class server {
    public:
        server(boost::asio::io_service& io_service, int port_number, request_dispatcher* request_dispatcher);

    private:
        void start_accept();
        void handle_accept(session* new_session, const boost::system::error_code& error);
        boost::asio::io_service& io_service_;
        tcp::acceptor acceptor_;
        request_dispatcher* request_dispatcher_;
};
