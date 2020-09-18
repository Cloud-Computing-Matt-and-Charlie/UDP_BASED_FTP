#include "UDP.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>


using namespace std;

UDP::UDP(char* dest_ip_address_in, char* listen_port_in, char* dest_port_in)
{
	memset(&this->hints, 0, sizeof(struct addrinfo));
	this->dest_port = dest_port_in;
	this->listen_port = listen_port_in;
	this->hints.ai_family = AF_INET;
	this->hints.ai_socktype = SOCK_DGRAM;
	this->hints.ai_flags = AI_PASSIVE; //try with and w/o this
	this->hints.ai_protocol = 0;
	this->packet_size = 15000; //bytes
	this->listen_buffer = new char[packet_size];
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
	if ((rv = getaddrinfo(dest_ip_address_in, dest_port_in, &hints, &this->dest_address)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		int init_rv = 1;
	}
	freeaddrinfo(servinfo);
	// if ((rv = getaddrinfo(NULL, dest_ip_address, &hints, dest_address)) != 0)
	// {
	// 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	// 	rv = 1;
	// }



}
int UDP::send(char* input_buffer)
{
	//input_buffer[message_size] = "\n";  NOTE**
	int numbytes;
	struct addrinfo* p = this->dest_address;
	if ((numbytes = sendto(this->sock_fd, input_buffer, this->packet_size, 0,
	                       p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}


	//printf("talker: sent %d bytes to %s\n", numbytes, (char*)p->ai_addr);
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

	printf("listener: packet contains \"%s\"\n", this->listen_buffer);
	return this->listen_buffer;

}

void UDP::setPacketSize(int new_packet_size)
{
	this->packet_size  = new_packet_size;
	delete [] this->listen_buffer;
	listen_buffer = new char[this->packet_size];
}

int bytes_to_int(unsigned char* byte_array, int num_bytes)
{
	int output = 0;
	int i;
	int j = 0;
	int k = 0;
	for (int x = num_bytes - 1; x >= 0; x--)
	{
		for (i = 7; i >= 0; i--)
		{
			output |= ((byte_array[x] << k * 8) & (1 << j));
			j++;
		}
		k++;
	}
	return output;
}

void int_to_bytes(unsigned int input, unsigned char** output, int& output_size)
{

	int bits = log2(input) + 1;
	if (!input)
	{
		output_size = 0;
		return;
	}
	int bytes = ceil((double)bits / 8);
	*output = new unsigned char[bytes];
	for ( int i = 0; i < bytes; i++)
	{
		(*output)[i] = (0xFF & input >> (8 * (bytes - 1 - i)));

	}
	output_size = bytes;
	return;

}




UDP::~UDP()
{
	close(this->sock_fd);
}


