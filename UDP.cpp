class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *A_address, *B_address;
	int sock_fd, init_rv = 0;
	map<string, addrinfo*> addresses;
	UDP()
	{

		memset(&this->hints, 0, sizeof this->hints);
		this->hints.ai_family = AF_INET;
		this->hints.ai_socktype = SOCK_DGRAM;
		int rv;
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(MYPORT), &(this->hints), &(this->servinfo))) != 0)
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
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(SERVERA), &hints, &A_address)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(SERVERB), &hints, &B_address)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		addresses.insert({"A", A_address});
		addresses.insert({"B", B_address});
		addresses.insert({"me", my_address});


	}
	int send(string server_ID, int message_size)
	{
		buf[message_size] = '\0';
		int numbytes;
		struct addrinfo* p = addresses[server_ID];
		if ((numbytes = sendto(this->sock_fd, buf, strlen(buf), 0,
		                       p->ai_addr, p->ai_addrlen)) == -1)
		{
			perror("talker: sendto");
			exit(1);
			return 1;
		}


		printf("talker: sent %d bytes to %s\n", numbytes, MYIPADDRESS);
		return 0;
	}
	int recieve()
	{
		int rv;
		int numbytes;
		struct sockaddr_storage their_addr;
		//char buf[MAXBUFLEN];
		socklen_t addr_len;
		char s[INET6_ADDRSTRLEN];
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sock_fd, buf, MAXBUFLEN - 1 , 0,
		                         (struct sockaddr*)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			exit(1);
			return 1;
		}


		printf("listener: got packet from %s\n",
		       inet_ntop(their_addr.ss_family,
		                 get_in_addr((struct sockaddr*)&their_addr),
		                 s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);

		return numbytes;
	}

	~UDP()
	{
		for (auto address : addresses)
		{
			freeaddrinfo(address.second);
		}
		close(sock_fd);
	}
};
