#include <iostream>
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/client.hpp"

typedef websocketpp::client<websocketpp::config::asio> client;

int main() {
    client c;
    try {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        c.init_asio();

        // Register our message handler
        c.set_message_handler([](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            std::cout << msg->get_payload() << std::endl;
        });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection("ws://localhost:9002", ec);

        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }

        // Note that connect really only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);

        // Start the ASIO io_service run loop
        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}