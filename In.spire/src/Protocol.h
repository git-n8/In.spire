#pragma once
#include "inpch.h"

class IN_SPIRE Buffer {
public:
    virtual size_t size() = 0;
    virtual void* data() = 0;
};

template<size_t SIZE>
class Packet : public Buffer {
public:
    size_t size() override {
        return SIZE;
    }

    void* data() override {
        return buffer;
    }

    uint8_t buffer[SIZE];
};

class Error {
public:
    virtual std::string message() = 0;
};

class ExceptionError : public Error {
public:
    ExceptionError(std::exception& e) : e(e) {}

    std::string message() override {
        return /*"[Exception]: " + */e.what();
    }

protected:
    std::exception& e;
};

using namespace std::placeholders;

class MessageHandler;

class IN_SPIRE Protocol {
public:
    Protocol() = default;

    class IN_SPIRE Connection {

    public:
        Connection(Protocol* protocol) : protocol(protocol), handler(nullptr) {}

        virtual bool isConnected() {
            return(!!this->handler);
        };

        virtual bool isReady() {
            return(!isConnected());
        };

        virtual void disconnect(bool force = false) = 0;

        template<typename DataType>
        inline void send(DataType data) {
            send(data, sizeof(DataType));
        }

        void receive();
        void send();

        inline void receive(void* buffer, size_t size);
        inline void send(const void* buffer, size_t size);

        void receive(std::function<void(size_t)> fn,
                    std::function<void(Error*)> err);

        void send(std::function<void(size_t)> fn,
                  std::function<void(Error*)> err);

        void receive(std::function<void(Error*)> err);
        void send(std::function<void(Error*)> err);

        void receive(std::function<void(size_t)> fn);
        void send(std::function<void(size_t)> fn);

        bool connect(MessageHandler* handler);
        void listenning(MessageHandler* handler);

    protected:
        virtual void receive(void* buffer, size_t size,
            std::function<void(size_t)> fn,
            std::function<void(Error*)> err) = 0;

        virtual void send(const void* buffer, size_t size,
            std::function<void(size_t)> fn,
            std::function<void(Error*)> err) = 0;

        void setHandler(MessageHandler* mh) {
            this->handler = mh;
        }

        Protocol* protocol;
        MessageHandler* handler;
    };

    class IN_SPIRE Endpoint {
    public:
        virtual Connection* getConnection(std::function<void(Connection*)> fn,
                                          std::function<void(Error*)> err) = 0;
    };

    Connection* connect(Endpoint* endpoint, MessageHandler* handler);

    virtual void listen(uint16_t port) = 0;

    virtual bool onStart() {
        return true;
    };

    virtual void onStop() {}
    virtual void accept(std::function<void(Connection*)> fn,
                        std::function<void(Error*)> err) = 0;
};
