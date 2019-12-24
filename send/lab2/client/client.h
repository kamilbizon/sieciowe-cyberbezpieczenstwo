#pragma once

#include <sys/socket.h>
#include <vector>
#include <memory>
#include <netinet/in.h>
#include <chrono>
#include "../logger.h"

class Client {
public:
	Client(sa_family_t sin_family,
		   in_port_t sin_port,
  		   const char* sin_addr);

	void createSocket(int domain, int type, int protocol);
	void bindSocket(sockaddr_in addr);
	void connect(int times, std::chrono::seconds delay, const std::string& format);

private:

	int sock_receiving;
	sockaddr_in address;
	sockaddr_in client_addr;
	Logger logger;
};

