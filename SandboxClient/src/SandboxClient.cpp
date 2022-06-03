#include "tcp.h"
#include "MessageHandler.h"

class PingHandler : public MessageHandler {
public:
	PingHandler() : packet(new Packet<9>()){};

	Protocol* getProtocol() override {
		return(new TCPProtocol());
	}

	Buffer* getBuffer() override {
		return packet;
	}

	void onSend(const void* buffer, size_t length, Protocol::Connection* con) override {

		std::cout << "Message sent [" << std::string((char*)buffer) << "] size [" << length << "]" << std::endl;
	}

	bool onReceive(void* buffer, size_t length, Protocol::Connection* con) override {
		
		std::cout << "Message back [" << std::string((char*)buffer) << "] size [" << length << "]" << std::endl;
		return(true);
	}

private:
	Packet<9>* packet;
};

int main()
{
	TCPProtocol* protocol = new TCPProtocol();
	Protocol::Connection* c = protocol->connect("127.0.0.1", 8888, new PingHandler());

	bool key[2] = { false, false };
	bool old_key[2] = { false, false };

	bool bQuit = false;
	
	std::string buffer = "PING TEST";

	while (!bQuit)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
		}
		
		if (c->isConnected())
		{
			if (key[0] && !old_key[0])
				c->send(buffer.c_str(), buffer.size());

			if (key[1] && !old_key[1]) bQuit = true;
		}
		else if(c->isReady())
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}

		for (int i = 0; i < 2; i++) old_key[i] = key[i];
	}

	return 0;
}