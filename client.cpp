/******************************************************//*
Creators: Matthew Pisini, Charles Bennett
Date: 9/19/20

Description:
Inputs: 
1. Port number to listen on
2. Path to the directory to save the incoming file

*//******************************************************/
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>


using namespace std;

int main(int argc, char const *argv[]) {

    fstream file;
    int socket_fd, client_length, input_length;
    struct sockaddr_in client, source;
    socklen_t source_size;


    if(argc<3){
        cout << "not enough arguments" << endl;
        exit(1);
    }

    if ( socket_fd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
        perror("Opening socket.");
    }
    client_length = sizeof(client);
    memset(&client,0,client_length);
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons(atoi(argv[1]));

    if ( bind(socket_fd, (struct sockaddr *)&client, client_length) < 0 ){
        close(socket_fd);
        perror("Binding socket.");
    }

    source_size = sizeof(struct sockaddr_in);
    file.open(argv[2], (fstream::binary | fstream::out) ); //write to file in binary

    while (1)
    {
        input_length = recvfrom(socket_fd,/*buffer*/,/*buffer_size*/, 0, (struct sockaddr *)&source, &source_size);
        if (input_length < 0) perror("revfrom");

    }
    



    return 0;

}