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
// #include <string.h>
#include <string>
// #include <vector>
// #include <queue>
#include <pthread.h>
#include "client.h"

#define HEADER_SIZE (2)
#define FIELD1_SIZE (2)
// #define DEST_PORT "10001"
// #define DEST_IP "192.168.86.152"
#define PACKET_SIZE (1500)
#define NUM_ACKS (5)
#define ACK_WINDOW (3)

using namespace std;

struct packet_content
{
    int payload_size;
    std::string payload;
};

/************************************** CONSTRUCTOR ****************************************/
client_listen::client_listen(char* dest_ip_address, char* listen_port, char* dest_port) :
    UDP(dest_ip_address, listen_port, dest_port)
{
    // char temp_char = '\0';
    // UDP* sessionAckUDP = new UDP(Client_IP_Address, Host_Port_Num, &temp_charA);


    int num_packets_expected = 0;
    this->packet_size = PACKET_SIZE;
    this->first_packet = false;
    // vector<int> packet_ID_list;
    queue< vector<char> > packet_queue;
    // queue< vector<char> > ACK_queue;
    vector< vector<char> > ACK_queue;
    queue< vector <char>> packet_ID_list;
    this->packet_ID_list_size = 0;
    pthread_mutex_init(&this->packet_lock, NULL);
    pthread_mutex_unlock(&this->packet_lock);
}
/**********************************************************************************************/

//add data to the array
void client_listen::map_add(int packet_number, vector<char> data)
{
    vector<char> payload = data;
    payload.erase(payload.begin(), payload.begin()+2);
    this->data_map.insert(std::pair<int, vector<char>>(packet_number, payload));
}
/**********************************************************************************************/

//print data array
void client_listen::print_data_map()
{
    vector<char>::iterator it;
    cout << "(packet ID, data)" << endl;
    for (const auto& x : this->data_map)
    {
        
        cout << x.first << ": ";// << x.second << endl;
        vector<char> items = x.second;
        for (it = items.begin(); it != items.end(); it++)
        {
            cout << *it;
        } 
        cout << endl;
    }
}
/**********************************************************************************************/

/************************************** PACKET PROCESSING ****************************************/
void client_listen::process_packet(vector<char> packet)
{
    //strip_header
    int packet_ID = strip_header(packet);

    //send payload (delete endline)
    map_add(packet_ID, packet);
    // pthread_mutex_unlock(&this->packet_lock);
}
//strips control info from header
int client_listen::strip_header(vector<char> data)
{
    unsigned char input[HEADER_SIZE];
    vector<char> ACK_input;
    // strncpy(input,data.c_str(), HEADER_SIZE);
    for (int i = 0; i < HEADER_SIZE; i++)
    {
        input[i] = data[i];
        ACK_input.push_back(data[i]);
    }
    int packet_ID = bytes_to_int(input, HEADER_SIZE);
    // cout << "strip header val: " << packet_ID << endl;
    this->packet_ID_list.push(ACK_input);
    this->packet_ID_list_size++;
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
/**********************************************************************************************/

/************************************** PACKET SENDING ****************************************/
void client_listen::create_ACK_packet()
{
    vector<char> output_packet;
    vector<char>::iterator it;
    for (int i = 0; i < NUM_ACKS; i++)
    {
        int j = 0;
        for (it = this->packet_ID_list.front().begin(); it != this->packet_ID_list.front().end(); it++)
        {
            output_packet.push_back(*it);
            // j = (unsigned char)it[0] | (unsigned char)it[1] << 8;
            // cout << j << endl;
        }
        this->packet_ID_list.pop();
    }
    this->ACK_queue.push_back(output_packet);
    // cout << "ACK packet size: " << output_packet.size() << endl;
}

void client_listen::send_ACKs(int index)
{
    
    // cout << this->packet_ID_list_size << " packets ACKed. " << this->ACK_queue.size() << " ACK packet ready to go!"<< endl;
    
    /* DEBUG
    cout << "output: ";
    int j = 0;
    for(int i = 0; i < this->ACK_queue.front().size(); i+=2)
    {   
        j = output[i] | output[i+1] << 8;
        cout << j << endl;
    }
    cout << endl;
    */

    // this->setPacketSize(NUM_ACKS);
    // vector<vector<char>>::iterator it;
    int temp;
    if(this->ACK_queue.size() < 5)
    {
        // temp = ACK_queue.size() - index;
        temp = (index + 1);
        index = 0;
    }
    else
    {
        temp = index;
        index = (index + 1) - ACK_WINDOW;
    }
    // int i = index;
    for (vector<vector<char>>::iterator it = (this->ACK_queue.begin()+index);
     it != (this->ACK_queue.begin() + index + temp); ++it)
    {
        unsigned char* output;
        output = (unsigned char *)vector_to_cstring(*it);
        // cout << "output: ";
        // int j = 0;
        // for(int i = 0; i < it->size(); i+=2)
        // {   
        //     // j = output[i] | output[i+1] << 8;
        //     unsigned char f[2] = {output[i],output[i+1]};
        //     j = bytes_to_int(f,2);
        //     cout << j << endl;
        // }
        // cout << endl;
        cout << "sending ACK Packet #: " << distance(this->ACK_queue.begin(),it) << endl;
        this->send((char *)output);
    }
}

void listener(char* dest_ip_address, char* listen_port, char* dest_port)
{
    client_listen client(dest_ip_address, listen_port, dest_port);
    int thread_num, byte_size;
    pthread_t processing_thread;
    // char * things = "123456789";
    // client.setPacketSize(9);
    // client.send(things);
    client.setPacketSize(NUM_ACKS*HEADER_SIZE);
    cout << "creating thread..." << endl;
    thread_num = pthread_create(&processing_thread, NULL, &empty_packet_queue, (void*)&client);
    int count = 0;
    while (1)
    {

        cout << "listening for packet..." << endl;
        char * temp = client.recieve(byte_size);
        // pthread_mutex_lock(&(client.packet_lock));
        // printf("%s", temp);
        vector<char> thread_buffer = cstring_to_vector(temp, byte_size);
        // string thread_buffer(temp);
        // cout << "byte_size: " << byte_size << endl;
        //first packet should be control
        // if (client.first_packet)
        // {
        //     // client.control_packet(thread_buffer);
        // }
        // else
        // {
        // for(auto it = thread_buffer.begin(); it != thread_buffer.end(); ++it)
        // {
        //     cout << *it;
        // }
        // cout << endl;


        // }
        cout << "thread_buffer size: " << thread_buffer.size() << endl;
        client.packet_queue.push(thread_buffer);
        // pthread_mutex_unlock(&(client.packet_lock));
    }

}

int main(int argc, char const* argv[])
{
    if (argc < 4)
    {
        cout << "Need more information: (DEST_IP, LISTEN_PORT, DEST_PORT)." << endl;
        exit(1);
    }
    // char temp_char = '\0';
    //destIP, LISTEN_portt, DESTPORT;
    char * DEST_IP = (char *)argv[1];
    char * LISTEN_PORT = (char *)argv[2];
    char * DEST_PORT = (char *)argv[3];

    listener(DEST_IP, LISTEN_PORT, DEST_PORT);
    // fstream file;
    // int socket_fd, client_length, input_length;
    // struct sockaddr_in client, source;
    // socklen_t source_size;


    return 0;

}

void* empty_packet_queue(void* input)
{
    class client_listen* client = static_cast<class client_listen*>(input);
    int index = 0;
    while (1)
    {
        // cout<<"packet queue size" << client->packet_queue.size() << endl;
        if (client->packet_ID_list.size() >= NUM_ACKS)
        {
            cout << "creating packet" << endl;
            client->create_ACK_packet();
            client->send_ACKs(index);
            index++;
        }

        if (client->packet_queue.size() > 0)
        {
            // pthread_mutex_lock(&(client->packet_lock));
            // pthread_t pthread_self(void);
            // cout << "processing packet w/ thread: " << &pthread_self << endl;
            vector<char> packet = client->packet_queue.front();
            // cout << "Packet contents: " << packet << endl;
            client->process_packet(packet);
            client->packet_queue.pop();
            // client->print_data_map();
        }
        else
        {
            // pthread_mutex_unlock(&(client->packet_lock));
            sleep(1);
        }
    }
};

char* vector_to_cstring(vector<char> input)
{
    char* output = new char[input.size()];
    for (int i = 0; i < input.size(); i++)
    {
        output[i] = input[i];
    }
    return output;
}
vector<char> cstring_to_vector(char* input, int size)
{
    vector<char> output(size);
    for (int i = 0; i < size; i++)
    {
        output[i] = input[i];
    }
    return output;
}

