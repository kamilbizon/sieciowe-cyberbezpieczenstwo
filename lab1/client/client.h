#pragma once

#include <sys/socket.h>
#include <vector>
#include <memory>
#include <netinet/in.h>

class Client {
public:
	Client(sa_family_t sin_family,
		   in_port_t sin_port,
  		   const char* sin_addr);

	void createSocket(int domain, int type, int protocol);
	void bindSocket(socklen_t address_len);
	void connect();

private:

	int sock_receiving;
	sockaddr_in address;
};

