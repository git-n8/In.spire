#pragma once
#include "Protocol.h"

#define ASIO_STANDALONE
#include <asio.hpp>

class IN_SPIRE IPProtocol : public Protocol {
public:
    bool onStart() override {

        // Launch the asio context in its own thread
        thread = std::thread([this]() { context.run(); });

        return Protocol::onStart();
    }

    void onStop() override {
        // Request the context to close
        context.stop();

        // Tidy up the context thread
        if (thread.joinable()) thread.join();
    }

    void post(std::function<void()> exec) {
        asio::post(context, exec);
    }

    class IPError : public Error {
    public:
        IPError() {};
        IPError(std::error_code error_code) : ec(error_code) {}
        void setErrorCode(std::error_code error_code) {
            ec = error_code;
        }

        std::string message() override {
            return(ec.message());
        }

    private:
        std::error_code ec;
    };

    IPError* error(std::error_code ec) {
        this->err.setErrorCode(ec);
        return(&this->err);
    }

protected:
    asio::io_context context;

private:
    IPError err;
    std::thread thread;
};

class IN_SPIRE TCPProtocol : public IPProtocol {

public:
    TCPProtocol() : acceptor(IPProtocol::context) {}

    void listen(uint16_t port) override {
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        this->acceptor.open(endpoint.protocol());
        this->acceptor.bind(endpoint);
        this->acceptor.listen();
    }

    class IN_SPIRE Connection : public Protocol::Connection {
    public:
        Connection(TCPProtocol* protocol, asio::ip::tcp::socket socket) : Protocol::Connection(protocol), tcp(protocol), 
                                                                          socket(std::move(socket)), ready(false) {}

        void receive(void* buffer, size_t size,
            std::function<void(size_t)> fn,
            std::function<void(Error*)> err) override
        {
            asio::async_read(this->socket, asio::buffer(buffer, size),
                [this, fn, err](std::error_code ec, std::size_t length) {
                    if (ec)
                        return err(tcp->error(ec));

                    fn(length);
                });
        }

        bool isConnected() override {
            return Protocol::Connection::isConnected() && socket.is_open();
        }

        bool isReady() override {
            return(ready);
        }

        void disconnect(bool force = false) override {
            if (!isConnected())
                return;

            if (force)
                return socket.close();

            tcp->post([this]() {
                socket.close();
                });
        }

        void send(const void* buffer, size_t size,
                    std::function<void(size_t)> fn,
                    std::function<void(Error*)> err)
        {
            if (!isConnected())
                return;

            tcp->post([this, buffer, size, fn, err]() {
                asio::async_write(this->socket, asio::buffer(buffer, size),
                    [this, fn, err, buffer](std::error_code ec, std::size_t length)
                    {
                        // asio has now sent the bytes - if there was a problem
                        // an error would be available...
                        if (ec) {
                            // ...asio failed to write the message, we could analyse why but 
                            // for now simply assume the connection has died by closing the
                            // socket. When a future attempt to write to this client fails due
                            // to the closed socket, it will be tidied up.
                            disconnect(true);
                            return err(tcp->error(ec));
                        }

                        fn(length);
                    });
                });
        }

        void connectTo(asio::ip::tcp::resolver::results_type& endpoints,
                       std::function<void(Protocol::Connection*)> fn,
                       std::function<void(Error*)> err) {
            // Request asio attempts to connect to an endpoint
            asio::async_connect(this->socket, endpoints,
                [this, err, fn](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                {
                    if (ec)
                        return err(new IPError(ec));

                    ready = true;
                    fn(this);
                });
        }

        asio::ip::tcp::socket socket;
        TCPProtocol* tcp;
        bool ready;
    };

    class IN_SPIRE Endpoint : public Protocol::Endpoint {
    public:
        Endpoint(TCPProtocol* protocol, std::string host, uint16_t port) {
            // Resolve hostname/ip-address into tangiable physical address
            this->endpoints = asio::ip::tcp::resolver(protocol->context).resolve(host, std::to_string(port));
            // Create connection
            this->con = new TCPProtocol::Connection(protocol, asio::ip::tcp::socket(protocol->context));
        }

        Protocol::Connection* getConnection(std::function<void(Protocol::Connection*)> fn,
                                            std::function<void(Error*)> err) override {
 
            // Tell the connection object to connect to server
            this->con->connectTo(this->endpoints, fn, err);
            return(this->con);
        }

        asio::ip::tcp::resolver::results_type endpoints;
        TCPProtocol::Connection* con;
    };

    Protocol::Connection* connect(std::string host, const uint16_t port, MessageHandler* handler) {
        return Protocol::connect(new Endpoint(this, host, port), handler);
    }

    void accept(std::function<void(Protocol::Connection*)> fn,
                std::function<void(Error*)> err) override {
        // Prime context with an instruction to wait until a socket connects. This
         // is the purpose of an "acceptor" object. It will provide a unique socket
         // for each incoming connection attempt

        acceptor.async_accept(
            [this, fn, err](std::error_code ec, asio::ip::tcp::socket socket)
            {
                // Triggered by incoming connection request
                if (ec)
                    return err(this->error(ec));

                // Display some useful(?) information
                std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

                // Create a new connection to handle this client 
                fn(new Connection(this, std::move(socket)));
            });
    }

protected:
    asio::ip::tcp::acceptor acceptor;
};

