#pragma once
#include "inpch.h"
#include "MessageHandler.h"
#include "Protocol.h"

#include <deque>


class IN_SPIRE Server {
public:
    Server(uint16_t port, MessageHandler* handler) : Server(port, handler->getProtocol(), handler) {}
    Server(uint16_t port, Protocol* protocol, MessageHandler* handler) : protocol(protocol), handler(handler) {
        protocol->listen(port);
    }

    ~Server() {
        stop();
    }

    bool start() {
        try
        {
            // Issue a task to the asio context - This is important
            // as it will prime the context with "work", and stop it
            // from exiting immediately. Since this is a server, we 
            // want it primed ready to handle clients trying to
            // connect.
            waitConnection();

            if (!protocol->onStart()) {
                std::cerr << "[SERVER] error on Protocol, server not started." << "\n";
                return false;
            }
        }
        catch (std::exception& e)
        {
            // Something prohibited the server from listening
            std::cerr << "[SERVER] Exception: " << e.what() << "\n";
            return false;
        }

        std::cout << "[SERVER] Started!\n";
        return true;
    }

    void waitConnection() {
        protocol->accept([this](Protocol::Connection* con) {

            // Give the user server a chance to deny connection
            if (con->connect(handler))
            {
                // Connection allowed, so add to container of new connections
                connections.push_back(con);

                std::cout << " Connection Approved\n";
            }
            else
            {
                std::cout << "[-----] Connection Denied\n";

                // Connection will go out of scope with no pending tasks, so will
                // get destroyed automagically due to the wonder of smart pointers
            }

            waitConnection();
            }, [this](Error* err) {
                // Error has occurred during acceptance
                std::cout << "[SERVER] New Connection Error: " << err->message() << "\n";

                handler->onError(err);

                // Prime the asio context with more work - again simply wait for
                // another connection...
                waitConnection();
            });
    }

    void stop()
    {
        protocol->onStop();

        // Inform someone, anybody, if they care...
        std::cout << "[SERVER] Stopped!\n";
    }

    MessageHandler* handler;
    Protocol* protocol;
    std::deque<Protocol::Connection*> connections;
};


