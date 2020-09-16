#include <UDP.h>
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

using namespace std; 

UDP::(char* dest_ip_address_in, char* listen_port_in, char* dest_port_in) : 
dest_ip_address_in{dest_ip_address_in}, listen_port{listen_port_in}, dest_port{dest_port_in}
{

	memset(&this->hints, 0, sizeof this->hints);
	this->hints.ai_family = AF_INET;
	this->hints.ai_socktype = SOCK_DGRAM;
	this->hints.ai_flags = AI_PASSIVE; //try with and w/o this 
	this->listen_buffer = new char[UDP_PACKET_SIZE]; 
	int rv;
	if ((rv = getaddrinfo(NULL, to_cstring(this->listen_port), &(this->hints), &(this->servinfo))) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		init_rv = 1;
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
		init_rv = 2;
	}
	freeaddrinfo(servinfo);
	if ((rv = getaddrinfo(NULL, dest_ip_address, &hints, dest_address)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		init_rv = 1;
	}



}
int UDP::send(char* intput_buffer, int message_size)
{
	//input_buffer[message_size] = "\n";  NOTE**
	
	int numbytes;
	struct addrinfo* p = this->dest_address;
	if ((numbytes = sendto(this->sock_fd, buf, message_size, 0,
			       p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
		return 1;
	}


	printf("talker: sent %d bytes to %s\n", numbytes, dest_ip_address);
	return 0;
}
char* UDP::recieve()
{
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	//char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	addr_len = sizeof their_addr;
	
	if ((numbytes = recvfrom(sock_fd, this->listen_buffer, MAXBUFLEN - 1 , 0,
				 (struct sockaddr*)&their_addr, &addr_len)) == -1)
	{
		perror("recvfrom");
		exit(1);
		return 1;
	}

	printf("listener: packet contains \"%s\"\n", this->listen_buffer);
	return this->listen_buffer; 

}

UDP::~UDP()
{
	for (auto address : addresses)
	{
		freeaddrinfo(address.second);
	}
	close(sock_fd);
}
