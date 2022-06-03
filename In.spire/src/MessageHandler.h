#pragma once
#include "Protocol.h"

class IN_SPIRE MessageHandler {
public:
    virtual Protocol* getProtocol() = 0;

    virtual bool onConnect(Protocol::Connection* con) {
        return true;
    };

    virtual Buffer* getBuffer() = 0;

    virtual void onError(Error* err) {}

    virtual bool onReceive(void* buffer, size_t length, Protocol::Connection* con) { return(true); }
    virtual void onSend(const void* buffer, size_t length, Protocol::Connection* con) {}
};

