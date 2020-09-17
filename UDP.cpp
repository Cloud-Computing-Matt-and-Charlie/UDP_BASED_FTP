#include "UDP.h"
#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std; 

UDP::UDP(char* dest_ip_address_in, char * listen_port_in, int dest_port_in)
{
	memset(&this->hints, 0, sizeof(struct addrinfo));
	this->dest_port = dest_port_in;
	this->listen_port = listen_port_in;
	this->hints.ai_family = AF_INET;
	this->hints.ai_socktype = SOCK_DGRAM;
	this->hints.ai_flags = AI_PASSIVE; //try with and w/o this
	this->hints.ai_protocol = 0;
	this->listen_buffer_size = 15000; //bytes
	this->listen_buffer = new char[listen_buffer_size]; 
	// this->hints->ai_family = AF_INET;
	// this->hints->ai_socktype = SOCK_DGRAM;
	// this->hints->ai_flags = AI_PASSIVE; //try with and w/o this
	// this->hints->ai_protocol = 0;
	// this->listen_buffer = new char[MAX_BUF_LEN];
	int rv;
	if ((rv = getaddrinfo(NULL, listen_port_in, &(this->hints), &(this->servinfo))) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		rv = 1;
	}
	for (my_address = servinfo; my_address != NULL; my_address = my_address->ai_next)
	{

		if ((sock_fd = socket(my_address->ai_family, my_address->ai_socktype,
				      my_address->ai_protocol)) == -1)
		{
			perror("listener: socket");
			continue;
		}

		if (::bind(sock_fd, my_address->ai_addr, my_address->ai_addrlen) == -1)
		{
			close(sock_fd);
			perror("listener: bind");
			continue;
		}


		break;
	}
	if (my_address == NULL)
	{
		fprintf(stderr, "listener: failed to bind socket\n");
		rv = 2;
	}
	freeaddrinfo(servinfo);
	// if ((rv = getaddrinfo(NULL, dest_ip_address, &hints, dest_address)) != 0)
	// {
	// 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	// 	rv = 1;
	// }



}
int UDP::send(char* intput_buffer, int message_size)
{
	//input_buffer[message_size] = "\n";  NOTE**
	
	int numbytes;
	struct addrinfo* p = this->dest_address;
	if ((numbytes = sendto(this->sock_fd, intput_buffer, message_size, 0,
			       p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}


	printf("talker: sent %d bytes to %s\n", numbytes, (char *)p->ai_addr);
	return 0;
}
char* UDP::recieve()
{
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	//char buf[MAXBUFLEN];
	//char * buffer = new char[buff_size]; 
	
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	addr_len = sizeof their_addr;
	
	if ((numbytes = recvfrom(sock_fd, this->listen_buffer, this->packet_size - 1 , 0,
				 (struct sockaddr*)&their_addr, &addr_len)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}

	printf("listener: packet contains \"%s\"\n", buffer);
	return this->listen_buffer; 

}

void UDP::SetPacketSize(int new_packet_size)
{
	this->packet_size  = new_packet_size; 
	del this->listen_buffer; 
	listen_buffer = new char[this->packet_size]; 
}
	

UDP::~UDP() = default; 


