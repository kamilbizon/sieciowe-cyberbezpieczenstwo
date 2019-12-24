#pragma once

#include <sys/socket.h>
#include <vector>
#include <memory>
#include <netinet/in.h>

class Server {
public:
	Server(sa_family_t sin_family,
		   in_port_t sin_port,
  		   const char* sin_addr);

	void createSocket(int domain, int type, int protocol);
	void bindSocket(socklen_t address_len);
	void listen();

private:

	int sock_receiving;
	std::vector<int> sokcs_receivers;
	int backlog = SOMAXCONN;
	sockaddr_in address;
};

