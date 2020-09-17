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
// #include <vector>
// #include <queue>
#include <pthread.h>
#include "client.h"

#define HEADER_SIZE (2)
#define FIELD1_SIZE (2)
#define DEST_PORT (10000)
#define DEST_IP "10.0.2.1"
#define PACKET_SIZE (1500)

using namespace std;

struct packet_content
{
    int payload_size;
    std::string payload;
};

client_listen::client_listen(char* dest_ip_address, char* listen_port, int dest_port) :
    UDP(dest_ip_address, listen_port, dest_port)
{
    char* data_array;
    int num_packets_expected, packet_size, array_size;
    bool first_packet = false;
    vector<int> packet_ID_list;
    queue<std::string> packet_queue;
    int packet_ID_list_size = 0;
    pthread_mutex_init(&this->packet_lock, NULL);
    pthread_mutex_unlock(&this->packet_lock);
}

//allocate initial array to input data
void client_listen::create_array(int packet_size, int num_packets)
{
    this->array_size = packet_size * num_packets;
    this->data_array = new char[array_size];
}

//add data to the array
void client_listen::array_add(std::string data, int packet_offset, int data_size)
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
int client_listen::strip_header(std::string data)
{

    int packet_ID = 0;
    for (int i = 0; i < HEADER_SIZE; i++)
    {
        packet_ID = (data[i] - (int)'0') + packet_ID * 10;
    }
    this->packet_ID_list.push_back(packet_ID);
    this->packet_ID_list_size++;
    cout << "packet ID: " << packet_ID << endl;
    return packet_ID;
}

void client_listen::control_packet(string data)
{
    //take control info --> need set format for it
    cout << data << endl;
    int field1 = 0;
    int field2 = 0;
    // field1 = (data[1]-(int)'0') | (data[0]-(int)'0') << 8;
    // field2 = (data[3]-(int)'0') | (data[2]-(int)'0') << 8;
    int j = FIELD1_SIZE * 8;
    for (int i = (FIELD1_SIZE * 8 - 1); i > 0 ; i--)
    {
        ;
        field1 |= (data[j / 8] << i) & (0x1 << i);
        j++;
    }

    this->packet_size = field1;
    this->num_packets_expected = field2;
    this->first_packet = false;
    cout << "packet size: " << this->packet_size << endl;
    cout << "num packets: " << this->num_packets_expected << endl;
}

void client_listen::process_packet(string packet, int size)
{
    //strip_header
    int packet_offset, packet_ID;
    packet_ID = strip_header(packet);
    //array_add
    packet_offset = packet_ID * this->packet_size;
    // pthread_mutex_lock(&this->packet_lock);
    packet_offset = size;
    array_add(packet, packet_offset, size);
    // pthread_mutex_unlock(&this->packet_lock);
}

void listener(const char* dest_ip_address, char* listen_port, int dest_port)
{
    client_listen client((char*)dest_ip_address, listen_port, dest_port);
    client.create_array(10, 20);
    int thread_num;
    pthread_t processing_thread;
    cout << "creating thread..." << endl;
    thread_num = pthread_create(&processing_thread, NULL, &empty_packet_queue, (void*)&client);
    while (1)
    {

        cout << "listening for packet..." << endl;
        string thread_buffer(client.recieve());
        client.packet_queue.push(thread_buffer);

        //first packet should be control
        if (client.first_packet)
        {
            client.control_packet(thread_buffer);
        }

    }

}

int main(int argc, char const* argv[])
{

    if (argc < 2)
    {
        cout << "need to supply listening port" << endl;
        exit(1);
    }
    listener((const char*)DEST_IP, (char*)argv[1], DEST_PORT);
    // fstream file;
    // int socket_fd, client_length, input_length;
    // struct sockaddr_in client, source;
    // socklen_t source_size;
    return 0;

}

void* empty_packet_queue(void* input)
{
    class client_listen* client = static_cast<class client_listen*>(input);
    int j = 1;

    while (1)
    {
        if (client->packet_queue.size() > 0)
        {
            // pthread_mutex_lock(&this->packet_lock);
            pthread_t pthread_self(void);
            cout << "processing packet w/ thread: " << &pthread_self << endl;
            std::string packet = client->packet_queue.front();
            cout << packet << endl;
            client->process_packet(packet, 5 * j);
            client->packet_queue.pop();
            client->print_data_array();
            // pthread_mutex_unlock(&this->packet_lock);
            j++;
        }
        else
        {
            sleep(1);
        }
    }
};