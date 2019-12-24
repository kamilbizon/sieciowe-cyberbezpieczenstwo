#pragma once

#include <sys/socket.h>
#include <vector>
#include <memory>
#include <netinet/in.h>
#include <openssl/dh.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <optional>

using DH_unique = std::unique_ptr<DH, decltype(&DH_free)>;
using BN_unique = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
using EVP_unique = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

class Server {
public:
	Server(sa_family_t sin_family,
		   in_port_t sin_port,
  		   const char* sin_addr);

	void createSocket(int domain, int type, int protocol);
	void bindSocket(socklen_t address_len);
	void listen();
	void accept();
	std::optional<std::vector<unsigned char>> recv();
	void send(std::vector<unsigned char> response);
	void send_to_be_or_not();

private:
	int encrypt(unsigned char *plaintext, int plaintext_len,
				unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
	int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            			unsigned char *iv, unsigned char *plaintext);

	int sock_receiving;
	std::vector<int> sokcs_receivers;
	int backlog = SOMAXCONN;
	sockaddr_in address;
	sockaddr_in sender_addr_in;
	int new_sock;

	DH_unique dh;
	std::optional<const BIGNUM*> pub_key{};
	std::optional<const BIGNUM*> priv_key{};
	std::vector<unsigned char> pub_key_binary;
};

