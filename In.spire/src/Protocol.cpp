#include "inpch.h"
#include "Protocol.h"
#include "MessageHandler.h"

void Protocol::Connection::receive() {
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        receive(buffer->data(), buffer->size());
    }
        
}

void Protocol::Connection::send() {
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        send(buffer->data(), buffer->size());
    }
}

void Protocol::Connection::receive(void* buffer, size_t size) {
    if (isConnected())
        receive(buffer, size,
            std::bind(&MessageHandler::onReceive, handler, buffer, _1, this),
            std::bind(&MessageHandler::onError, handler, _1));
}

void Protocol::Connection::send(const void* buffer, size_t size) {
    if (isConnected())
        send(buffer, size,
            std::bind(&MessageHandler::onSend, handler, buffer, _1, this),
            std::bind(&MessageHandler::onError, handler, _1));
}

void Protocol::Connection::receive(std::function<void(size_t)> fn, std::function<void(Error*)> err)
{
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        receive(buffer->data(), buffer->size(), fn, err);
    }
        
}

void Protocol::Connection::send(std::function<void(size_t)> fn, std::function<void(Error*)> err)
{
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        send(buffer->data(), buffer->size(), fn, err);
    }
}

void Protocol::Connection::receive(std::function<void(Error*)> err)
{
    if (isConnected()){
        Buffer* buffer = this->handler->getBuffer();
        receive(buffer->data(), buffer->size(),
                std::bind(&MessageHandler::onReceive, this->handler, this->handler->getBuffer(), _1, this), err);
    }
}

void Protocol::Connection::send(std::function<void(Error*)> err)
{
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        send(buffer->data(), buffer->size(),
            std::bind(&MessageHandler::onReceive, this->handler, this->handler->getBuffer(), _1, this), err);
    }
}

void Protocol::Connection::receive(std::function<void(size_t)> fn)
{
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        receive(buffer->data(), buffer->size(),
                fn, std::bind(&MessageHandler::onError, this->handler, _1));
    }
}

void Protocol::Connection::send(std::function<void(size_t)> fn)
{
    if (isConnected()) {
        Buffer* buffer = this->handler->getBuffer();
        send(buffer->data(), buffer->size(),
            fn, std::bind(&MessageHandler::onError, this->handler, _1));
    }
}


void Protocol::Connection::listenning(MessageHandler* handler)
{
    Buffer* buffer = handler->getBuffer();
    void* data = buffer->data();

    receive(data, buffer->size(),
        [handler, this, data](size_t length) {
        if (handler->onReceive(data, length, this))
            listenning(handler);
        },
        std::bind(&MessageHandler::onError, handler, _1)
    );
}

bool Protocol::Connection::connect(MessageHandler* handler)
{
    if (!handler->onConnect(this))
        return(false);

    this->setHandler(handler);

    if (!isConnected())
        return(false);

    // And very important! Issue a task to the connection's
    // asio context to sit and wait for bytes to arrive!
    listenning(handler);
    return(true);
}

Protocol::Connection* Protocol::connect(Endpoint* endpoint, MessageHandler* handler) {

    try {
        // Resolve hostname/ip-address into tangiable physical address
        // Start Context Thread
        if (onStart())
            // Create connection
            return endpoint->getConnection(
                [handler](Connection* con) { con->connect(handler); },
                std::bind(&MessageHandler::onError, handler, _1)
            );
    }
    catch (std::exception& e) {
        handler->onError(new ExceptionError(e));
    }

    return(nullptr);
}