#include "client.h" 
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <openssl/md5.h>


Client::Client(sa_family_t sin_family,
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
	BN_bn2bin(*pub_key, static_cast<unsigned char*>(pub_key_binary.data()));
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
	auto connect_status = ::connect(sock_receiving, reinterpret_cast<const sockaddr*>(&address), sizeof(address));

	if (connect_status == -1) {
		std::cerr << "connect status = " << connect_status << std::endl;
		return;
	}

	std::cout << "connect status = " << connect_status << std::endl;	
}

void Client::send(std::vector<unsigned char> response) {
	auto send_status = ::send(sock_receiving, response.data(), response.size(), 0);

	if (send_status == -1) {
		std::cerr << "send status = " << send_status << std::endl;
		return;
	}

	std::cout << "send status = " << send_status << std::endl;
}

std::optional<std::vector<unsigned char>> Client::recv() {
	std::vector<unsigned char> buffer{};
	buffer.resize(1024);
	auto recv_status = ::recv(sock_receiving, buffer.data(), buffer.size(), 0);

	if (recv_status == -1) {
		std::cerr << "recv status = " << recv_status << std::endl;
		return std::nullopt;
	}

	std::cout << "recv status = " << recv_status << std::endl;

	buffer.resize(recv_status);
	return buffer;
}

void Client::send_key() {
	// send my public key
	std::vector<unsigned char> message{pub_key_binary.begin(), pub_key_binary.end()};
	std::cout << "pub key: " << message.size() << std::endl;
	send(message);

	// recv server public key
	auto server_key = recv();
	// cast public serv key to BIGNUMBER
	BN_unique pub_key_serv_bn(BN_bin2bn(static_cast<const unsigned char*>(server_key->data()), server_key->size(), NULL), BN_free);

	// compute mixed key with my pub key and serv pub key
	std::vector<unsigned char> computed_mixed_key{};
	computed_mixed_key.resize(8192);
	DH_compute_key(static_cast<unsigned char*>(computed_mixed_key.data()), pub_key_serv_bn.get(), dh.get());

	// recv encrypted msg
	auto ciphertext = recv();

	unsigned char iv[128] = {0};
	std::vector<unsigned char> plaintext;
	plaintext.resize(8192);
	auto plaintext_len = decrypt(ciphertext->data(), ciphertext->size(),
								static_cast<unsigned char*>(computed_mixed_key.data()),
					            iv, plaintext.data());
	std::cout << "plaintext_len: " << plaintext_len << std::endl;
	plaintext.resize(plaintext_len);
	std::string plaintext_string{plaintext.begin(), plaintext.end()};
	std::cout << "dec msg: " << plaintext_string << std::endl;

	// chceck if correct
	unsigned char* plaintext_md5 = MD5(reinterpret_cast<const unsigned char*>(plaintext_string.c_str()),
									   plaintext_string.size(), NULL);
	auto md5_len = strlen(reinterpret_cast<const char*>(plaintext_md5));
	std::vector<unsigned char> plaintext_md5_vec(plaintext_md5, plaintext_md5+md5_len);

	auto server_md5 = recv();

	if(not (plaintext_md5_vec == *server_md5))
		std::cerr << "message incorrect\n";
	else std::cout << "message correct\n"; 
}

// source: https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
int Client::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
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
