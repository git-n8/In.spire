#include "Server.h"
#include "tcp.h"

class EchoHandler : public MessageHandler {
public:
    EchoHandler() : packet(new Packet<9>()) {}

    Protocol* getProtocol() override {
        return new TCPProtocol();
    }

    bool onReceive(void* buffer, size_t length, Protocol::Connection* con) override {
        con->send( buffer, length );
        return(true);
    }

    void onSend(const void* buffer, size_t length, Protocol::Connection* con) override {
        std::cout << "echo back [" << std::string((char*)buffer) << "] " << "size [" << length << "]" << std::endl;
    }

    void onError(Error* error) override {
        std::cout << "Connection error [" << error->message() << "]" << std::endl;
    }

    Buffer* getBuffer() {
        return packet;
    }

    Packet<9>* packet;
    Protocol::Connection* con;
};

int main(int argc, char** argv) {

    std::cout << "Booting Sandbox Server..." << std::endl;

    Server* server = new Server(8888, new EchoHandler());
    server->start();

    while (true);
    return 0;
}
