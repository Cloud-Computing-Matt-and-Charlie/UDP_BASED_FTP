#ifndef UDP_H
#define UDP_H

#define PACKET_SIZE 10000

class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *dest_address;
	int sock_fd; 
	int rv;
	int listen_port; 
	int dest_port; 
	int desig; 
	
	map<string, addrinfo*> addresses;
	UDP(char* dest_ip_address, int listen_port, int dest_port); 
	int send(char* buffer, int message_size); 
	char * recieve(int buff_size); 
	~UDP(); 
};

#endif
