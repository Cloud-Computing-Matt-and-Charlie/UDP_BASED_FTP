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
#include <pthread.h>
#include "UDP.h"
#include "client.h"

#define HEADER_SIZE (2)
#define DEST_PORT (10000)
#define DEST_IP '10.0.2.1'
#define PACKET_SIZE (1500)

using namespace std;


client_listen::(char* dest_ip_address, int listen_port, int dest_port)
{
    UDP listen_on(char* dest_ip_address, int listen_port, int dest_port);
    char *data_array;
    int num_packets_expected, packet_size, array_size;
    bool first_packet = true;
    vector<int> packet_ID_list;
    int packet_ID_list_size = 0;
}

//allocate initial array to input data
void client_listen::create_array(int packet_size, int num_packets)
{
    this->array_size = packet_size*num_packets;
    this->data_array = new char[array_size];
}
//deallocate array
void client_listen::destroy_array()
{
    delete [] this->data_array;
}

//add data to the array
void client_listen::array_add(char * data, int packet_offset, int data_size)
{

    for (int i = HEADER_SIZE; i < data_size; i++)
    {
        this->data_array[packet_offset + i] = data[i];
    }
}

//print data array
void client_listen::print_data_array()
    {
        for (int i = 0; i < this->array_size; i++)
        {
            cout << i << ": " << this->data_array[i] << endl;
        }
    }

//strips control info from header
int client_listen::strip_header(char * data)
{

    int packet_ID = 0;
    for (int i = 0; i < HEADER_SIZE; i++)
    {
        packet_ID |= (data[i] - (int)'0') << 8*i;
    }
    this->packet_ID_list.push_back(packet_ID);
    this->packet_ID_list_size++;
    cout << "packet ID: " << packet_ID << endl;
    return packet_ID;
}

//takes packet and calls necessary functions to handle processing
void client_listen::process_packet(char * data, int size)
{
    if (this->first_packet)
    {
        //take control info --> need set format for it
        int field1 = 0;
        int field2 = 0;
        field1 = (data[1]-(int)'0') | (data[0]-(int)'0') << 8;
        field2 = (data[3]-(int)'0') | (data[2]-(int)'0') << 8;
        this->packet_size = field1;
        this->num_packets_expected = field2;
        this->first_packet = false;
    }
    //strip_header
    int packet_offset, packet_ID;
    packet_ID = strip_header(data);

    //array_add
    packet_offset = packet_ID * this->packet_size;
    array_add(data, packet_offset, size);
}

client_listen::~client_listen()
{

}


class client_send
{
    void construct_packet()
    {

    }

    void send_packet()
    {

    }
};

void listener(char* dest_ip_address, int listen_port, int dest_port)
{
    client_listen client(dest_ip_address, listen_port, dest_port);
    char * buf;
    while (1)
    {

        buf = client.recieve(PACKET_SIZE);
        //spawn thread to handle packet processing
        delete [] buf;


    }
    

}

int main(int argc, char const *argv[]) {

    // client_listen client;

    // client.create_array(10,20);
    // cout << sizeof(client.data_array) << endl;


    // char *input = "01hello this is the server";

    // client.process_packet(input);
    // client.print_data_arrray();

    if(argc<2){
        cout << "need to supply listening port" << endl;
        exit(1);
    }
    listener((char *)DEST_IP, atoi(argv[1]), DEST_PORT);
    // fstream file;
    // int socket_fd, client_length, input_length;
    // struct sockaddr_in client, source;
    // socklen_t source_size;


   

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