/******************************************************//*
Creators: Matthew Pisini, Charles Bennett
Date: 9/19/20
Description:
Inputs:
1. Destination IP address
2. Port # to listen on
3. Destination port # to send to
4. Name of the file to save incoming data to
*//******************************************************/
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <pthread.h>
#include <chrono>
#include "client.h"

#define HEADER_SIZE (4)                            //Total number of bytes per packet in the header 
#define PACKET_SIZE (3000)                         //Optional parameter for use
#define NUM_ACKS (300)                              //Number of ACKs per packet (each ACK is 2 byte packet ID) 
#define ACK_WINDOW (5)                             //Sliding window of duplicate ACK transmissions

/*************** CONTROL FIELDS *******************/

#define FIELD1_SIZE (2)                            //Packet Size
#define FIELD2_SIZE (2)                            //# of Packets in Transmission
#define NUM_CONTROL_FIELDS (2)                     //# Fields in control header
#define NUM_PACKETS_EXPECTED (32226)               //Hardcoded Packet Size (comment if control packet in use)
int control_field_array[NUM_CONTROL_FIELDS];       //Array to store the decoded control fields
int control_field_sizes[NUM_CONTROL_FIELDS]        //Define sizes of control fields
    = {FIELD1_SIZE, FIELD1_SIZE};
std::chrono::time_point<std::chrono::system_clock> FIRST_PACKET_TIME, LAST_PACKET_TIME;
double BANDWIDTH;
double FILE_SIZE = NUM_PACKETS_EXPECTED * PACKET_SIZE;
int LARGEST_PACKET = 0;
/**************************************************/

using namespace std;

client_listen::~client_listen() {};
/************************************** CONSTRUCTOR ****************************************/
client_listen::client_listen(char* dest_ip_address, char* listen_port, char* dest_port, char* output_file) :
    UDP(dest_ip_address, listen_port, dest_port)
{
    this->file_name = output_file;
    this->num_packets_expected = NUM_PACKETS_EXPECTED;
    this->num_packets_received = 0;
    this->first_packet = false;
    queue< vector<char> > packet_queue;
    queue< vector<char> > packets_for_write;
    vector< vector<char> > ACK_queue;
    queue< vector <char>> packet_ID_list;
    this->packet_ID_list_size = 0;
    pthread_mutex_init(&this->packet_lock, NULL);
    pthread_mutex_unlock(&this->packet_lock);
    pthread_mutex_init(&this->write_lock, NULL);
    pthread_mutex_unlock(&this->write_lock);
}
/**********************************************************************************************/

/************************************** PAYLOAD MANAGEMENT (MAP) ****************************************/
//add data to the array
// void client_listen::map_add(int packet_number, vector<char> data)
// {
//     vector<char> payload = data;
//     payload.erase(payload.begin(), payload.begin() + 2);
//     if (!this->data_map.count(packet_number))
//     {
//         this->num_packets_received++;
//     }
//     this->data_map.insert(std::pair<int, vector<char>>(packet_number, payload));
// }
/**********************************************************************************************/
//print data array
// void client_listen::print_data_map()
// {
//     vector<char>::iterator it;
//     cout << "(packet ID, data)" << endl;
//     for (const auto& x : this->data_map)
//     {

//         cout << x.first << ": ";// << x.second << endl;
//         vector<char> items = x.second;
//         for (it = items.begin(); it != items.end(); it++)
//         {
//             cout << *it;
//         }
//         cout << endl;
//     }
// }
/**********************************************************************************************/

/************************************** PACKET PROCESSING ****************************************/
void client_listen::process_packet(vector<char> packet)
{
    //strip_header
    // int packet_ID = this->strip_header(packet);
    unsigned char input[HEADER_SIZE];
    vector<char> ACK_input;
    for (int i = 0; i < HEADER_SIZE; i++)
    {
        input[i] = (unsigned char)packet[i];
        ACK_input.push_back(packet[i]);
    }
    int packet_ID = bytes_to_int(input, HEADER_SIZE);
    if (packet_ID > LARGEST_PACKET) LARGEST_PACKET = packet_ID;
    //add payload to map and packet_ID_list if it is a unique packet ID
    if (!this->data_map.count(packet_ID))
    {
        this->num_packets_received++;
        cout << "Packet ID: " << packet_ID << endl;
        this->packet_ID_list.push(ACK_input);
        this->packet_ID_list_size++;
        // cout << "Total packets received: " << this->num_packets_received << endl;
        pthread_mutex_lock(&this->write_lock);
        this->packets_for_write.push(packet);
        pthread_mutex_unlock(&this->write_lock);
        // this->data_map.insert(std::pair<int, vector<char>>(packet_ID, payload));
        this->data_map.insert(std::pair<int,char> (packet_ID, ' '));
    }
}
//strips control info from header
int client_listen::strip_header(vector<char> &data)
{
    unsigned char input[HEADER_SIZE];
    vector<char> ACK_input;
    for (int i = 0; i < HEADER_SIZE; i++)
    {
        input[i] = (unsigned char)data[i];
        ACK_input.push_back(data[i]);
    }
    int packet_ID = bytes_to_int(input, HEADER_SIZE);
    return packet_ID;
}
//processing of control packet (first transmission)
void client_listen::control_packet(vector<char> data)
{
    int offset = 0;
    for (int i = 0; i < NUM_CONTROL_FIELDS; i++)
    {
        unsigned char input[control_field_sizes[i]];
        for (int j = 0; j < control_field_sizes[i]; j++)
        {
            input[j] = data[j + offset];
        }
        control_field_array[i] = bytes_to_int(input, control_field_sizes[i]);
        offset += control_field_sizes[i];
    }

    cout << "FIELD1: " << control_field_array[0] << endl;
    cout << "FIELD2: " << control_field_array[1] << endl;

    this->packet_size = control_field_array[0];             //FIELD1 = packet size
    this->num_packets_expected = control_field_array[1];    //FIELD2 = num packets
    this->first_packet = false;
}
/**********************************************************************************************/

/************************************ PACKET TRANSMISSION **************************************/
//Bundle ACKS into single payload for transmission
void client_listen::create_ACK_packet(int ACK_packet_size)
{
    vector<char> output_packet;
    vector<char>::iterator it;
    if (this->packet_ID_list.size() < ACK_packet_size)
    {
        cout << "error over indexing ack creation vector" << endl;
    }
    for (int i = 0; i < ACK_packet_size; i++)
    {
        int j = 0;
        for (it = this->packet_ID_list.front().begin(); it != this->packet_ID_list.front().end(); it++)
        {
            output_packet.push_back(*it);
        }
        this->packet_ID_list.pop();
    }
    // cout << "packet_ID_list.size(): " << this->packet_ID_list.size() << endl;
    this->ACK_queue.push_back(output_packet);
}

void client_listen::send_ACKs(int index)
{
    int temp;
    if (this->ACK_queue.size() <= ACK_WINDOW)
    {
        temp = (index + 1);
        index = 0;
    }
    else
    {
        temp = index + 1;
        index = (index + 1) - ACK_WINDOW;
    }
    for (vector<vector<char>>::iterator it = (this->ACK_queue.begin() + index);
            it != (this->ACK_queue.begin() + temp); ++it)
    {
        unsigned char* output;
        output = (unsigned char*)vector_to_cstring(*it);
        //  DEBUG
        // cout << "output: ";
        // int j = 0;
        // for(int i = 0; i < it->size(); i+=HEADER_SIZE)
        // {
        //     // j = output[i] | output[i+1] << 8;
        //     unsigned char f[4] = {output[i],output[i+1],output[i+2],output[i+3]};
        //     j = bytes_to_int(f,4);
        //     cout << j << endl;
        // }
        // cout << endl;
        // cout << "sending ACK Packet #: " << distance(this->ACK_queue.begin(), it) << endl;
        // for (int i = 0; i < it->size(); i++)
        // {
        //     this->send_buffer[i] = output[i];
        // }
        this->send((char*)output);
        delete [] output;
    }
}

void listener(char* dest_ip_address, char* listen_port, char* dest_port, char* output_file)
{
    client_listen client(dest_ip_address, listen_port, dest_port, output_file);
    int thread_num1,thread_num2, byte_size, rc;
    bool first_packet = true;
    void* status;
    pthread_t processing_thread, writing_thread;
    pthread_attr_t attr, attr2;
    pthread_attr_init(&attr);
    pthread_attr_init(&attr2);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_JOINABLE);
    client.setSendPacketSize(NUM_ACKS * HEADER_SIZE);
    client.setPacketSize(PACKET_SIZE);
    thread_num1 = pthread_create(&processing_thread, &attr, empty_send_queue, (void*)&client);
    thread_num2 = pthread_create(&writing_thread, &attr2, empty_data_queue, (void*)&client);
    pthread_attr_destroy(&attr);
    int byte_total =0;
    int packet_count =0 ;
    while (1)
    {
        char* temp = client.recieve(byte_size);
        if (first_packet)
        {
            FIRST_PACKET_TIME = std::chrono::system_clock::now();
            first_packet = false;
        }
        pthread_mutex_lock(&client.packet_lock);
        byte_total += byte_size;
        packet_count++;
        cout << "packet count: " << packet_count << endl;
        vector<char> thread_buffer = cstring_to_vector(temp, byte_size);
        // client.packet_queue.push(thread_buffer);
        client.process_packet(thread_buffer);
        pthread_mutex_unlock(&client.packet_lock);
        if (client.num_packets_received >= client.num_packets_expected) //have all the packets
        {
            LAST_PACKET_TIME = std::chrono::system_clock::now();
            rc = pthread_join(processing_thread, &status);
            if (rc)
            {
                std::cout << "ERRROR: joining processing thread" << std::endl;
                exit(1);
            }
            rc = pthread_join(writing_thread, &status);
            if (rc)
            {
                std::cout << "ERRROR: joining writing thread" << std::endl;
                exit(1);
            }
            cout << "byte amount: " << byte_total << endl;
            auto TOTAL_TIME = std::chrono::duration_cast<std::chrono::milliseconds>(LAST_PACKET_TIME - FIRST_PACKET_TIME).count();
            std::cout << "Transmission completed in " << TOTAL_TIME << " milliseconds." << std::endl;
            BANDWIDTH = (FILE_SIZE / TOTAL_TIME)*1000;
            std::cout << "Bandwidth: " << BANDWIDTH << " bytes/sec " << std::endl;
            cout << "LARGEST PACKET: " << LARGEST_PACKET << endl;
            pthread_exit(NULL);
        }
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
        // cout << "thread_buffer size: " << thread_buffer.size() << endl;

        //process packet

    }

}

int main(int argc, char const* argv[])
{
    if (argc < 5)
    {
        std::cout << "Need more information: (DEST_IP, LISTEN_PORT, DEST_PORT, OUTPUT_FILE)." << endl;
        exit(1);
    }
    char* DEST_IP = (char*)argv[1];
    char* LISTEN_PORT = (char*)argv[2];
    char* DEST_PORT = (char*)argv[3];
    char* output_file = (char*)argv[4];
    listener(DEST_IP, LISTEN_PORT, DEST_PORT, output_file);
    // file_reader();

    return 0;
}

void file_reader()
{
    int count = 0;
    int vec_size = 200;
    char * buff = new char[vec_size];
    ifstream file("test_file.txt", ios::in);
    file.seekg(0, ios::end);
    size_t file_len = file.tellg();
    int total_bytes = (int)file_len;
    cout << "File length is: " << total_bytes << endl;
    ofstream outfile;
    outfile.open("output.txt");
    long index;
    int remainder = 0;
    while(count*vec_size <= total_bytes)
    {
        remainder = total_bytes - count*vec_size;
        if (remainder < vec_size)
        {
            vec_size = remainder;
            cout << "Changed vec size" << endl;
        }
        file.read(buff, vec_size);
        // for (int i =0; i < vec_size; i++)
        // {
        //     cout << buff[i];
        // }
        index = count*vec_size;
        cout << "Index: " << index << endl;
        cout << "Veczise: " << vec_size << endl;
        outfile.seekp(index);
        outfile.write(buff, vec_size);
        count++;
    }
    file.close();
    outfile.close();
    cout << "DONE" << endl;
}

void * empty_data_queue(void* input)
{
    client_listen* client = static_cast<client_listen*>(input);
    std::cout << "Data writing thread created" << std::endl;
    char * raw_data;
    unsigned char temp_buf[HEADER_SIZE];
    ofstream file;
    file.open(client->file_name, ios::binary);
    int vec_size;
    long index;
    int subtract_sum = 0;
    int wrist_band_sum = 0;
    map<int,int> wristband_map;
    int total_bytes = 0;
    int packet_total = 0;
    int wrist_band_FF = 0;
    while(1)
    {
        if(client->packets_for_write.size() > 0)   //data to be written
        {
            raw_data = vector_to_cstring(client->packets_for_write.front());
            for (int i = 0; i < HEADER_SIZE; i++)
            {
                temp_buf[i] = (unsigned char)raw_data[i];
            }
            int packet_ID = bytes_to_int(temp_buf, HEADER_SIZE);
            vec_size = client->packets_for_write.front().size();
            total_bytes += vec_size - HEADER_SIZE;
            if (vec_size < PACKET_SIZE)
            {
                cout << "vec size: " << vec_size << endl;
                wrist_band_sum += ((PACKET_SIZE - HEADER_SIZE) - (vec_size - HEADER_SIZE));
                cout << "wrist_band_sum: " << wrist_band_sum << endl;
                if(packet_total != (NUM_PACKETS_EXPECTED-1)) wristband_map.insert(std::pair<int,int>(packet_ID, wrist_band_sum));
            }
            if (!wristband_map.empty())
            {
                for (const auto& x : wristband_map)
                {
                    if (packet_ID > x.first)
                    {
                        subtract_sum = x.second;
                        wrist_band_FF = 1;
                    } 
                }
                if(!wrist_band_FF)
                {
                    subtract_sum = 0;
                }
                wrist_band_FF = 0;
            }
            index = packet_ID*(PACKET_SIZE - HEADER_SIZE) - subtract_sum;
            file.seekp(index);
            cout << "writing packet: " << packet_ID << endl;
            file.write(raw_data + HEADER_SIZE, (vec_size-HEADER_SIZE));
            packet_total++;
            pthread_mutex_lock(&client->write_lock);
            client->packets_for_write.pop();
            // cout << "Packet total: " << packet_total << endl;
            pthread_mutex_unlock(&client->write_lock);
        }
        if ((client->packets_for_write.size() == 0)&& (packet_total == NUM_PACKETS_EXPECTED))
        {
            cout << "wrote " << total_bytes << " to file from ";
            cout << packet_total << " packets" << endl;
            cout << "killing writing thread" << endl;
            file.close();
            pthread_exit(NULL);
        }

    }

}

void* empty_send_queue(void* input)
{
    client_listen* client = static_cast<client_listen*>(input);
    int index = 0;
    cout << "Packet processing thread created" << endl;
    while (1)
    {
        // if (client->packet_queue.size() > 0)
        // {
        //     pthread_mutex_lock(&client->packet_lock);
        //     vector<char> temp = client->packet_queue.front();
        //     client->packet_queue.pop();
        //     pthread_mutex_unlock(&client->packet_lock);
        //     client->process_packet(temp);
        // }
        // cout<<"packet queue size" << client->packet_queue.size() << endl;
        if (client->packet_ID_list.size() >= NUM_ACKS)
        {
            // pthread_mutex_lock(&client->packet_lock);
            cout << "creating packet" << endl;
            // cout << "creating packet" << endl;
            client->create_ACK_packet(client->packet_ID_list.size());
            client->send_ACKs(index);
            index++;
            // pthread_mutex_unlock(&client->packet_lock);
        }
        if (client->num_packets_received >= client->num_packets_expected)
        {
            // pthread_mutex_lock(&client->packet_lock);
            cout << "creating final packet" << endl;
            client->create_ACK_packet(client->packet_ID_list.size());
            for (int i = 0; i < ACK_WINDOW; i++)
            {
                client->send_ACKs(index);   //make sure last packet is sent ACK_WINDOW times
            }
            // pthread_mutex_unlock(&client->packet_lock);
            // pthread_t pthread_self(void);
            cout << "killing processing thread" << endl;
            pthread_exit(NULL);
        }
        // if (client->packet_queue.size() > 0) //packets still to be processed
        // {
        //     //Have not received all of the unique packets wee expect tot receive
        //     // if (client->num_packets_received < client->num_packets_expected)
        //     // {
        //     //     // pthread_mutex_lock(&client->packet_lock);
        //     //     // vector<char> packet = client->packet_queue.front();
        //     //     // client->process_packet(packet);
        //     //     // client->packets_for_write.push(packet);
        //     //     // client->packet_queue.pop();
        //     //     // pthread_mutex_unlock(&client->packet_lock);
        //     // }

        //     //Received all of the packets we expected to receive --> done processing
        //     if ( (client->num_packets_received >= client->num_packets_expected) && !(client->first_packet) )
        //     {
        //         cout << "creating final packet" << endl;
        //         client->create_ACK_packet(client->packet_ID_list.size());
        //         for (int i = 0; i < ACK_WINDOW; i++)
        //         {
        //             client->send_ACKs(index);   //make sure last packet is sent ACK_WINDOW times
        //         }
        //         // pthread_t pthread_self(void);
        //         cout << "killing processing thread" << endl;
        //         pthread_exit(NULL);
        //     }
        // }
        // else
        // {
        //     sleep(0.01); //optional sleep parameter
        // }
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

long vector_bytes_to_int(vector<char> input, long start, long end)
{
	//MSB ... LSB
	long output = 0;
	long temp;
	for (long i = start; i <= end; i++)
	{

		temp = (unsigned char)input[i];
		temp = temp << (8 * (end - i));
		output |= temp;
		temp = 0;
	}
	return output;
}
