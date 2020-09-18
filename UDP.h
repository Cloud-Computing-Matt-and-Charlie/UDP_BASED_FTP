#ifndef UDP_H
#define UDP_H

#include <string>
#include <map>
#include <netdb.h>
#include <string>

// #define PACKET_SIZE 10000

struct addrinfo;

class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *dest_address;
	int sock_fd;
	int rv;
	char* listen_port;
	char* dest_port;
	int desig;
	char* listen_buffer;
	int packet_size;

	UDP(char* dest_ip_address_in, char* listen_port_in, char* dest_port_in);
	int send(char* buffer);
	char* recieve(int& bytes);

	void setPacketSize(int new_packet_size);
	~UDP();
};

int bytes_to_int(unsigned char* byte_array, int num_bytes);

void int_to_bytes(unsigned int input, unsigned char** output, int& output_size);





#endif

