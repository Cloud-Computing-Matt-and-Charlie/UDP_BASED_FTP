class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *A_address, *B_address;
  int sock_fd; 
  int rv; 
	map<string, addrinfo*> addresses;
	UDP(); 
	int send(string server_ID, int message_size); 
	int recieve(); 
	~UDP(); 
};
