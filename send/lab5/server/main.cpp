#include "server.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <string>

using namespace std;

constexpr int PORT = 65280;

int main() {
	Server serv(AF_INET, PORT, "127.0.0.1");

	serv.createSocket(AF_INET, SOCK_STREAM, 0);
	serv.bindSocket(sizeof(sockaddr));
	serv.listen();
	serv.accept();

	serv.send_to_be_or_not();

	return 0;
}
