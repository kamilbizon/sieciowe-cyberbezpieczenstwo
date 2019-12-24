#include "client.h" 
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

Client::Client(sa_family_t sin_family,
			   in_port_t sin_port,
  			   const char* sin_addr) {
	address.sin_family = sin_family;
	address.sin_port = htons(sin_port);
	address.sin_addr = in_addr{ inet_addr(sin_addr) };
}

void Client::createSocket(int domain, int type, int protocol) {
	sock_receiving = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_receiving == -1) {
		std::cerr << "socket status = " << sock_receiving << std::endl;
		return;
	}

	std::cout << "socket status = " << sock_receiving << std::endl;
}

void Client::bindSocket(socklen_t address_len) {
	auto result = bind(sock_receiving, reinterpret_cast<const sockaddr*>(&address), address_len);

	if (result == -1) {
		std::cerr << "bind status = " << result << std::endl;
		return;
	}

	std::cout << "bind status = " << result << std::endl;
}

void Client::connect() {

	while(1) {
		auto connect_status = ::connect(sock_receiving, reinterpret_cast<const sockaddr*>(&address), sizeof(address));

		if (connect_status == -1) {
			std::cerr << "connect status = " << connect_status << std::endl;
			return;
		}

		std::cout << "connect status = " << connect_status << std::endl;

		while(1) {
			std::string response{"Dziala\0"};
			auto send_status = send(sock_receiving, response.c_str(), response.size(), 0);

			if (send_status == -1) {
				std::cerr << "send status = " << send_status << std::endl;
				return;
			}

			std::cout << "send status = " << send_status << std::endl;


			char buffer[100];

			auto recv_status = recv(sock_receiving, buffer, sizeof(buffer), 0);

			if (recv_status == -1) {
				std::cerr << "recv status = " << recv_status << std::endl;
				return;
			}

			std::cout << "recv status = " << recv_status << std::endl;
			std::cout << buffer << std::endl;

			return;
		}
		return;
	}
}