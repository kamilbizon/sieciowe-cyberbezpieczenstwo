#include "client.h" 
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <array>
#include <algorithm>

constexpr int ERROR_STATUS = -1;

Client::Client(sa_family_t sin_family,
			   in_port_t sin_port,
  			   const char* sin_addr) {
	address.sin_family = sin_family;
	address.sin_port = htons(sin_port);
	address.sin_addr = in_addr{ inet_addr(sin_addr) };
}

void Client::createSocket(int domain, int type, int protocol) {
	sock_receiving = socket(AF_INET, SOCK_STREAM , 0);
	if (sock_receiving == -1) {
		std::cerr << "socket status = " << sock_receiving << std::endl;
		return;
	}
}

void Client::bindSocket(sockaddr_in addr) {
	client_addr = addr;

	auto result = bind(sock_receiving, reinterpret_cast<const sockaddr*>(&client_addr), sizeof(client_addr));

	if (result == -1) {
		std::cerr << "bind status = " << result << std::endl;
		return;
	}
}

void Client::connect(int times, std::chrono::seconds delay, const std::string& format) {

	while(1) {
		auto connect_status = ::connect(sock_receiving, reinterpret_cast<const sockaddr*>(&address), sizeof(address));

		if (connect_status == -1) {
			logger.logStatusError("connect", connect_status);

			if(errno != EINPROGRESS)
				return;
		}

		for(auto i=0; i < times; ++i) {
			std::string msg{format};
			auto send_status = send(sock_receiving, msg.c_str(), msg.length(), 0);

			if (send_status == -1) {
				logger.logStatusError("send", send_status);
				return;
			}

			std::array<char, 1024> buffer;
			auto recv_status = ::recv(sock_receiving, buffer.data(), buffer.max_size(), 0);

			if (recv_status == ERROR_STATUS) {
				logger.logStatusError("recv", recv_status);
				return;
			}

			auto string_end = std::find(buffer.begin(), buffer.end(), '\0');
			std::string resp_msg{buffer.begin(), string_end};

			std::cout << resp_msg << std::endl;

		    std::this_thread::sleep_for(delay);
		}
		return;
	}
}