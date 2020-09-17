#ifndef UDP_H
#define UDP_H

#include <string>
#include <map>
#include <netdb.h>

// #define PACKET_SIZE 10000

struct addrinfo;

class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *dest_address;
	int sock_fd; 
	int rv;
	char * listen_port; 
	int dest_port; 
	int desig; 
	
	std::map<std::string, addrinfo*> addresses;
	UDP(char* dest_ip_address, char * listen_port, int dest_port); 
	int send(char* buffer, int message_size); 
	char * recieve(int buff_size); 
	~UDP(); 
};

#endif
