#include "client.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>

using namespace std;

constexpr int PORT = 65280;

int main() {
	Client client(AF_INET, PORT, "127.0.0.1");

	client.createSocket(AF_INET, SOCK_STREAM, 0);
	client.connect();
	client.send_key();
	
	return 0;
}
