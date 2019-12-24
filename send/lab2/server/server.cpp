#include "server.h" 

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <array>

#include "getTime.h"

constexpr int ERROR_STATUS = -1;

Server::Server(sa_family_t sin_family,
			   in_port_t sin_port,
  			   const char* sin_addr) {
	address.sin_family = sin_family;
	address.sin_port = htons(sin_port);
	address.sin_addr = in_addr{ inet_addr(sin_addr) };
}

void Server::createSocket(int domain, int type, int protocol) {
	sock_receiving = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sock_receiving == ERROR_STATUS) {
		logger.logStatusError("socket", sock_receiving);
		return;
	}

	pollfd_list.push_back(pollfd{sock_receiving, POLLIN});
	sock_sockData.push_back({sock_receiving, address});


	logger.logStatus("socket", sock_receiving);
}

void Server::bindSocket(socklen_t address_len) {
	auto result = bind(sock_receiving, reinterpret_cast<const sockaddr*>(&address), address_len);

	if (result == ERROR_STATUS) {
		logger.logStatusError("bind", result);
		perror("bind");
		return;
	}

	std::cout << "bind status = " << result << std::endl;
}

void Server::listen() {
	auto listen_status = ::listen(sock_receiving, backlog);

	if (listen_status == ERROR_STATUS) {
		logger.logStatusError("listen", listen_status);
		return;
	}

	std::cout << "listen status = " << listen_status << std::endl;

	startPoll();
}

void Server::startPoll() {
	while(1) {
		int poll_result = poll(pollfd_list.data(), pollfd_list.size(), -1);

		if (poll_result == ERROR_STATUS) {
			logger.logStatusError("poll", poll_result);
			return;
		}

		for(const auto& pollfd_element : pollfd_list) {

			if (pollfd_element.revents & POLLIN) {

				if(pollfd_element.fd == sock_receiving)
					accept();
				else
					recvAndSend(pollfd_element.fd);
			}
		}
	}
}

void Server::accept() {
	sockaddr_in sender_addr_in;
	socklen_t sender_addr_len = sizeof(sockaddr_in);

	auto new_sock = ::accept(sock_receiving, reinterpret_cast<sockaddr*>(&sender_addr_in), &sender_addr_len);

	if(new_sock != ERROR_STATUS) {
		std::cout << "accept status = " << new_sock;
		pollfd_list.push_back(pollfd{new_sock, POLLIN});
		sock_sockData.push_back({new_sock, sender_addr_in});
	}
	else {
		logger.logStatusError("accept", new_sock);
		return;
	}

	std::cout << " address = " << inet_ntoa(sender_addr_in.sin_addr)
		  	  << " port = " << ntohs(sender_addr_in.sin_port)
		  	  << " protocol = " << sender_addr_in.sin_family << std::endl;
}

void Server::recvAndSend(int receiving_socket) {
	auto msg = recv(receiving_socket);
	if (msg)
		send(receiving_socket, *msg); 
}

std::optional<std::string> Server::recv(int receiving_socket) {
	std::array<char, 1024> buffer;

	auto recv_status = ::recv(receiving_socket, buffer.data(), buffer.max_size(), 0);

	if (recv_status == ERROR_STATUS) {
		logger.logStatusError("recv", recv_status);
		return std::nullopt;
	}
	if (recv_status == 0) {
		std::cout << "recv status = " << recv_status << " close socket: " << receiving_socket << std::endl;
		
		close(receiving_socket);

		auto pollfd_elem = std::find_if(pollfd_list.begin(), pollfd_list.end(), [receiving_socket](const auto& el) { return el.fd == receiving_socket; });
		pollfd_list.erase(pollfd_elem);

		auto sock_data_elem = std::find_if(sock_sockData.begin(), sock_sockData.end(), [receiving_socket](const auto& el) { return el.first == receiving_socket; });
		sock_sockData.erase(sock_data_elem);

		return std::nullopt;
	}

	auto string_end = std::find(buffer.begin(), buffer.end(), '\0');

	std::string msg{buffer.begin(), string_end};


	auto sock_data_elem = std::find_if(sock_sockData.begin(), sock_sockData.end(), [receiving_socket](const auto& el) { return el.first == receiving_socket; });
	std::cout << "recv from"
		      << " address = " << inet_ntoa(sock_data_elem->second.sin_addr)
		  	  << " port = " << ntohs(sock_data_elem->second.sin_port)
			  << " msg: "<< msg << std::endl;

	return msg;
}

void Server::send(int receiving_socket, const std::string& buffer) {
	auto response = getTime(buffer.c_str());
	response += '\0';

	auto send_status = ::send(receiving_socket, response.c_str(), response.length(), 0);

	if (send_status == ERROR_STATUS) {
		logger.logStatusError("send", send_status);
		return;
	}
}

