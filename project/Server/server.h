#ifndef SERVER_H_
#define SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define PAYLOAD_SIZE 8192 // Tuneable. must match the transmitter side
#define HEADER 2

class ESE532_Server {
public:

	//
	int setup_server(int avg_payload_size);

	//
	int get_packet(unsigned char* buffer);

protected:

	//
	int sockfd;

	// payload_size passed in from cli
	int payload_size;

	// addresss information
	struct sockaddr_in servaddr;

	//
	socklen_t server_len = sizeof(servaddr);

	int packets_read;

};

#endif
