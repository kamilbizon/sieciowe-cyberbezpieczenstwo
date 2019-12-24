#include "client.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <chrono>

using namespace std;

constexpr int PORT = 65289;

int main(int argc, char *argv[]) {
	std::string port{argv[1]};
	std::string format{argv[2]};
	std::string delay{argv[3]};
	std::string times{argv[4]};

	Client client{AF_INET, PORT, "127.0.0.1"};

	client.createSocket(AF_INET, SOCK_STREAM, 0);
	client.bindSocket(sockaddr_in{AF_INET, htons(stoi(port)), in_addr{ inet_addr("127.0.0.1") } });
	client.connect(stoi(times), chrono::seconds{std::stoi(delay)}, format);

	return 0;
}
