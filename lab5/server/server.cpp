#include "server.h" 
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <openssl/dh.h>

Server::Server(sa_family_t sin_family,
			   in_port_t sin_port,
  			   const char* sin_addr) {
	address.sin_family = sin_family;
	address.sin_port = htons(sin_port);
	address.sin_addr = in_addr{ inet_addr(sin_addr) };
}

void Server::createSocket(int domain, int type, int protocol) {
	sock_receiving = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_receiving == -1) {
		std::cerr << "socket status = " << sock_receiving << std::endl;
		return;
	}

	std::cout << "socket status = " << sock_receiving << std::endl;
}

void Server::bindSocket(socklen_t address_len) {
	auto result = bind(sock_receiving, reinterpret_cast<const sockaddr*>(&address), address_len);

	if (result == -1) {
		std::cerr << "bind status = " << result << std::endl;
		return;
	}

	std::cout << "bind status = " << result << std::endl;
}

void Server::listen() {
	while(1){
		auto listen_status = ::listen(sock_receiving, backlog);

		if (listen_status == -1) {
			std::cerr << "listen status = " << listen_status << std::endl;
			return;
		}

		std::cout << "listen status = " << listen_status << std::endl;


		sockaddr_in sender_addr_in;
		socklen_t sender_addr_len = sizeof(sockaddr_in);

		auto new_sock = accept(sock_receiving, reinterpret_cast<sockaddr*>(&sender_addr_in), &sender_addr_len);

		if(new_sock != -1) {
			std::cout << "accept status = " << new_sock << std::endl;
			sokcs_receivers.push_back(new_sock);
		}
		else {
			std::cerr << "accept status = " << new_sock << std::endl;
			return;
		}

		while(1) {
			char buffer[10000];

			auto recv_status = recv(new_sock, buffer, sizeof(buffer), 0);

			if (recv_status == -1) {
				std::cerr << "recv status = " << recv_status << std::endl;
				return;
			}

			std::cout << "recv status = " << recv_status << std::endl;
			std::cout << "address = " << inet_ntoa(sender_addr_in.sin_addr) << " port = " << ntohs(sender_addr_in.sin_port) << " protocol = " << sender_addr_in.sin_family << std::endl;
			std::cout << buffer << std::endl;

			std::string response{"Re: "};
			response += buffer;
			auto send_status = send(new_sock, response.c_str(), response.size(), 0);

			if (send_status == -1) {
				std::cerr << "send status = " << send_status << std::endl;
				return;
			}

			std::cout << "send status = " << send_status << std::endl;
			
			return;
		}
		return;
	}
}