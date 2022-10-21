// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "server.h"

#define PORT	 8091
#define HEADER 2

// basic
int ESE532_Server::setup_server(int avg_blocksize) {

	printf("setting up sever...\n");

	//
	int opt = 1;

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
			sizeof(opt)))
		perror("sockopt");

	// Filling server information
	//
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr))
			< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	//
	server_len = sizeof(servaddr);

	//
	blocksize = avg_blocksize;

	printf("server setup complete!\n");

	return 0;
}

int ESE532_Server::get_packet(unsigned char* buffer) {
	int bytes_read = recvfrom(sockfd, (void *) buffer, blocksize + HEADER, 0,
			(struct sockaddr *) &servaddr, &server_len);
	packets_read++;
	// crash
	if (bytes_read < 0) {
		perror("recvfrom failed!");
		//assert(bytes_read > 0);
	}

	return bytes_read;
}
