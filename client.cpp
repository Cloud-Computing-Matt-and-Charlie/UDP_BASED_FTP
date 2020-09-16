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
#include <string.h>
#include <vector>

#define HEADER_SIZE (2)
using namespace std;

class client_listen
{
    public:
    char *data_array;
    int num_packets_expected, packet_size, array_size;
    bool first_packet = true;
    vector<int> packet_ID_list;

    //allocate initial array to input data
    void create_array(int packet_size, int num_packets)
    {
        this->array_size = packet_size*num_packets;
        this->data_array = new char[array_size];
    }
    //deallocate array
    void destroy_array()
    {
        delete [] this->data_array;
    }

    //add data to the array
    void array_add(char * data, int packet_offset, int data_size)
    {

        for (int i = HEADER_SIZE; i < data_size; i++)
        {
            this->data_array[packet_offset + i] = data[i];
        }
    }

    //print data array
    void print_data_arrray()
    {
        for (int i = 0; i < this->array_size; i++)
        {
            cout << i << ": " << this->data_array[i] << endl;
        }
    }

    //strips control info from header
    void strip_header(char * data, int &packet_ID)
    {

        int temp = 0;
        for (int i = 0; i < HEADER_SIZE; i++)
        {
            temp |= (data[i] - (int)'0') << 8*i;
            // (data[1]-(int)'0') | (data[0]-(int)'0') << 8;
        }
        packet_ID = temp;

        cout << "packet ID: " << packet_ID << endl;

    }

    //takes packet and calls necessary functions to handle processing
    void process_packet(char * data, int size)
    {
        if (first_packet)
        {
            //take control info --> need set format for it
            // this->packet_size = ;
            first_packet = false;
        }
        //strip_header
        int packet_offset, packet_ID;
        strip_header(data, packet_ID);

        //array_add
        packet_offset = packet_ID * this->packet_size;
        array_add(data, packet_offset, size);
    }

};

class client_send
{
    void construct_packet()
    {

    }

    void send_packet()
    {

    }
};

int main(int argc, char const *argv[]) {

    client_listen client;

    client.create_array(10,20);
    cout << sizeof(client.data_array) << endl;


    char *input = "01hello this is the server";

    client.process_packet(input);
    client.print_data_arrray();


    // fstream file;
    // int socket_fd, client_length, input_length;
    // struct sockaddr_in client, source;
    // socklen_t source_size;


    // if(argc<3){
    //     cout << "not enough arguments" << endl;
    //     exit(1);
    // }

    // if ( socket_fd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
    //     perror("Opening socket.");
    // }
    // client_length = sizeof(client);
    // memset(&client,0,client_length);
    // client.sin_family = AF_INET;
    // client.sin_addr.s_addr = INADDR_ANY;
    // client.sin_port = htons(atoi(argv[1]));

    // if ( bind(socket_fd, (struct sockaddr *)&client, client_length) < 0 ){
    //     close(socket_fd);
    //     perror("Binding socket.");
    // }

    // source_size = sizeof(struct sockaddr_in);
    // // file.open(argv[2], (fstream::binary | fstream::out) ); //write to file in binary

    // while (1)
    // {
    //     input_length = recvfrom(socket_fd,/*buffer*/,/*buffer_size*/, 0, (struct sockaddr *)&source, &source_size);
    //     if (input_length < 0) perror("revfrom");

    // }
    



    return 0;

}