#include "server.h" 
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <openssl/md5.h>
#include "../sendUnrealiable.h"

Server::Server(sa_family_t sin_family,
			   in_port_t sin_port,
  			   const char* sin_addr) 
			   : dh(DH_get_2048_256(), DH_free) {
	address.sin_family = sin_family;
	address.sin_port = htons(sin_port);
	address.sin_addr = in_addr{ inet_addr(sin_addr) };

	if(not DH_generate_key(dh.get()))
		std::cerr << "no generate";

	pub_key = DH_get0_pub_key(dh.get());
	pub_key_binary.resize(BN_num_bytes(*pub_key));
	BN_bn2bin(*pub_key, reinterpret_cast<unsigned char*>(pub_key_binary.data()));
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
	auto listen_status = ::listen(sock_receiving, backlog);

	if (listen_status == -1) {
		std::cerr << "listen status = " << listen_status << std::endl;
		return;
	}

	std::cout << "listen status = " << listen_status << std::endl;
}

void Server::accept() {
	socklen_t sender_addr_len = sizeof(sockaddr_in);

	new_sock = ::accept(sock_receiving, reinterpret_cast<sockaddr*>(&sender_addr_in), &sender_addr_len);

	if(new_sock != -1) {
		std::cout << "accept status = " << new_sock << std::endl;
		sokcs_receivers.push_back(new_sock);
	}
	else {
		std::cerr << "accept status = " << new_sock << std::endl;
		return;
	}
}

std::optional<std::vector<unsigned char>> Server::recv() {
	std::vector<unsigned char> buffer{};
	buffer.resize(1024);
	auto recv_status = ::recv(new_sock, buffer.data(), buffer.size(), 0);

	if (recv_status == -1) {
		std::cerr << "recv status = " << recv_status << std::endl;
		return std::nullopt;
	}

	std::cout << "recv status = " << recv_status << std::endl;
	std::cout << "address = " << inet_ntoa(sender_addr_in.sin_addr) << " port = " << ntohs(sender_addr_in.sin_port) << " protocol = " << sender_addr_in.sin_family << std::endl;

	buffer.resize(recv_status);
	return buffer;
}

void Server::send(std::vector<unsigned char> response) {
	auto send_status = ::send(new_sock, response.data(), response.size(), 0);

	if (send_status == -1) {
		std::cerr << "send status = " << send_status << std::endl;
		return;
	}

	std::cout << "send status = " << send_status << std::endl;
}

void Server::send_to_be_or_not() {
	// receive client key
	auto client_key = recv();
	// cast public client key to BIGNUMBER
	BN_unique pub_key_client_bn(BN_bin2bn(static_cast<const unsigned char*>(client_key->data()), client_key->size(), NULL), BN_free);

	// send my public key
	std::vector<unsigned char> message{pub_key_binary.begin(), pub_key_binary.end()};
	std::cout << "pub key: " << message.size() << std::endl;
	send(message);

	// compute mixed key with my pub key and clinet pub key
	std::vector<unsigned char> computed_mixed_key{};
	computed_mixed_key.resize(8192);
	DH_compute_key(reinterpret_cast<unsigned char*>(computed_mixed_key.data()), pub_key_client_bn.get(), dh.get());



	// send encrypted msg
	unsigned char iv[128] = {0};
	std::vector<unsigned char> ciphertext;
	ciphertext.resize(8192);
	std::string answer("To be or not to be");
	auto ciphertext_len = encrypt(reinterpret_cast<unsigned char*>(answer.data()), answer.size(),
								  static_cast<unsigned char*>(computed_mixed_key.data()),
								  iv, ciphertext.data());
	std::cout << "ciphertext_len: " << ciphertext_len << std::endl;
	ciphertext.resize(ciphertext_len);
	std::string ciphertext_string{ciphertext.begin(), ciphertext.end()};
	std::cout << "enc msg: " << ciphertext_string << std::endl;

	sendUnreliable(new_sock, ciphertext.data(), ciphertext.size());

	// check if correct
	auto originalmd5 = MD5((unsigned char*)answer.c_str(), answer.size(), NULL);
	auto md5_len = strlen(reinterpret_cast<const char*>(originalmd5));
	send(std::vector<unsigned char>(originalmd5, originalmd5+md5_len));
	
	std::cout << originalmd5 << std::endl;
}

// source: https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
int Server::encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        std::cerr << "EVP_CIPHER_CTX_new error\n";
    }

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        std::cerr << "EVP_EncryptInit_ex error\n";
    }

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        std::cerr << "EVP_EncryptUpdate error\n";
    }

    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        std::cerr << "EVP_EncryptFinal_ex error\n";
    }

    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

// source: https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
int Server::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            		unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        std::cerr << "EVP_CIPHER_CTX_new error\n";
    }
    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        std::cerr << "EVP_DecryptInit_ex error\n";
    }
    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        std::cerr << "EVP_DecryptUpdate error\n";
    }
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        std::cerr << "EVP_DecryptFinal_ex error\n";
    }
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
