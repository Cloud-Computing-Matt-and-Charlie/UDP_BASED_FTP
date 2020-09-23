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
//#include <cstring>

#define UDP_PRINT_SEND 0
#define UDP_PRINT_RECV 1
//#include <cstring>


using namespace std;

UDP::UDP(char* dest_ip_address_in, char* listen_port_in, char* dest_port_in)
{
	/********* RECEIVER SETUP **********/

	memset(&this->hints, 0, sizeof(struct addrinfo));
	this->dest_port = dest_port_in;
	this->listen_port = listen_port_in;
	this->hints.ai_family = AF_INET;
	this->hints.ai_socktype = SOCK_DGRAM;
	this->hints.ai_flags = AI_PASSIVE; //try with and w/o this
	this->hints.ai_protocol = 0;
	this->packet_size = 15000; //bytes
	this->listen_buffer = new char[packet_size];
	int rv;
	if ((rv = getaddrinfo(NULL, listen_port_in, &(this->hints), &(this->servinfo))) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}
	for (my_address = servinfo; my_address != NULL; my_address = my_address->ai_next)
	{

		if ((this->sock_fd = socket(my_address->ai_family, my_address->ai_socktype,
		                            my_address->ai_protocol)) == -1)
		{
			perror("listener: socket");
			continue;
		}

		if (::bind(this->sock_fd, my_address->ai_addr, my_address->ai_addrlen) == -1)
		{
			close(sock_fd);
			perror("listener: bind");
			continue;
		}
		// if (this->dest_port[0] == '\0')
		// {
		// 	if (::bind(sock_fd, my_address->ai_addr, my_address->ai_addrlen) == -1)
		// 	{
		// 		close(sock_fd);
		// 		perror("listener: bind");
		// 		continue;
		// 	}
		// }
		break;
	}
	cout << "sock_fd: " << sock_fd << endl;
	struct sockaddr_in* ipv4 = (struct sockaddr_in*)my_address->ai_addr;
	void* addr = &(ipv4->sin_addr);
	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(my_address->ai_family, addr, ipstr, sizeof ipstr);
	printf("sock IP: %s\n", ipstr);

	if (my_address == NULL)
	{
		perror("listener failed to bind to socket.");
	}
	freeaddrinfo(servinfo);


	// if (this->dest_port[0] != '\0')
	// {
	// 	if (my_address == NULL)
	// 	{
	// 		fprintf(stderr, "listener: failed to bind socket\n");
	// 		rv = 2;
	// 	}
	// 	if ((rv = getaddrinfo(dest_ip_address_in, dest_port_in, &hints, &this->dest_address)) != 0)
	// 	{
	// 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	// 		int init_rv = 1;
	// 	}
	// }

	/********* SENDER SETUP **********/
	int check_UDP;
	struct addrinfo* p;
	memset(&hints_sender, 0, sizeof hints_sender);
	hints_sender.ai_family = AF_INET;
	hints_sender.ai_socktype = SOCK_DGRAM;
	if ((check_UDP = getaddrinfo(dest_ip_address_in, dest_port_in, &hints_sender, &p)) != 0)
	{
		perror("Error getting sending address.");
	}
	for (this->dest_address = p; this->dest_address != NULL; this->dest_address = this->dest_address->ai_next)
	{
		if ( (this->send_sock_fd = socket(this->dest_address->ai_family, this->dest_address->ai_socktype, this->dest_address->ai_protocol)) == -1)
		{
			perror("UDP socket creaton failure for sending");
			continue;
		}
		break;
	}
	cout << "send_sock_fd: " << send_sock_fd << endl;
	// ipstr = "";
	inet_ntop(this->dest_address->ai_family, addr, ipstr, sizeof ipstr);
	printf("send sock IP: %s\n", ipstr);
	if (this->dest_address == NULL)
	{
		perror("sender failed to bind to socket.");
	}
	//	freeaddrinfo(this->dest_address); DONT FREE THIS



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
	// struct addrinfo* p = this->dest_address;
	if ((numbytes = sendto(this->send_sock_fd, input_buffer, this->send_packet_size, 0,
	                       this->dest_address->ai_addr, this->dest_address->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}
	if (UDP_PRINT_SEND)
	{
		printf("SENDING: ");
		for (int i = 0; i < this->send_packet_size; i++)
		{
			printf("%c", input_buffer[i]);
		}
		//printf("talker: sent %d bytes to %s\n", numbytes, (char*)p->ai_addr);
	}

<<<<<<< HEAD
	// printf("SENDING: ");
	// for (int i = 0; i < this->send_packet_size; i ++)
	// {
	// 	printf("%c", input_buffer[i]);
	// }
	printf("talker: sent %d bytes\n", numbytes);
=======
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e
	return 0;
}

char* UDP::recieve(int& bytes)
{
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	//char buf[MAXBUFLEN];
	//char * buffer = new char[buff_size];

	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sock_fd, this->listen_buffer, this->packet_size, 0,
	                         (struct sockaddr*)&their_addr, &addr_len)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}
<<<<<<< HEAD
	// printf("RECEIVING: ");
	// for (int i = 0; i < this->packet_size; i ++)
	// {
	// 	printf("%c", this->listen_buffer[i]);
	// }
	bytes = numbytes;
	printf("listener: num bytes %d bytes: %d\n", numbytes, bytes);
=======
	if (UDP_PRINT_RECV)
	{
		printf("RECEIVING: ");
		for (int i = 0; i < numbytes; i++)
		{
			printf("%c", this->listen_buffer[i]);
		}
		bytes = numbytes;
		printf("listener: num bytes %d %d \n", numbytes, this->packet_size);
	}

>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e
	return this->listen_buffer;
}

void UDP::setPacketSize(int new_packet_size)
{
	this->packet_size  = new_packet_size;
	delete [] this->listen_buffer;
	this->listen_buffer = new char[this->packet_size];
}

void UDP::setSendPacketSize(int new_packet_size)
{
	this->send_packet_size = new_packet_size;
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
		// (*output)[i] = (0xFF & input >> (8 * (bytes - i - 1)));
		(*output)[i] = (0xFF & (input >> (8 * (bytes - i - 1))));

	}
	output_size = bytes;
	return;

}




UDP::~UDP()
{
	close(this->sock_fd);
}

