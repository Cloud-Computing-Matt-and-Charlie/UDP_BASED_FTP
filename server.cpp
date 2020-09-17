/******************************************************//*
Creators: Matthew Pisini, Charles Bennett
Date: 9/19/20

Description:
Inputs:
1. IP address of the client
2. Port number of the client
3. Path to the directory of the file to be sent to client.

*//******************************************************/

#include<iostream>
#include "UDP.h"
#include "packet_dispenser.h"
#include<iostream>
#include<cmath>
#include<vector>
#include <fstream>
#include <streambuf>
#define SEQUENCE_BYTE_NUM 2
#define NUM_SENDING_THREADS 4
int PACKET_SIZE = 16;
pthread_mutex_t print_lock;
void* sender_thread_function(void* input_param);
int get_sequence_number(string packet);
char* readFileBytes(const char* name, int& length);
void read_from_file(const char* file_name, int packet_size,
                    int sequencing_bytes, vector<string>& output);

struct ThreadArgs
{
	ThreadArgs(pthread_t* self_in, int id_in, UDP* myUDP_in,
	           PacketDispenser* myDispenser_in) :
		id{id_in}, myUDP{myUDP_in}, myDispenser{myDispenser_in}, self{self_in}
	{};
	pthread_t* self;
	int id;
	UDP* myUDP;
	PacketDispenser* myDispenser;
};




void* sender_thread_function(void* input_param)
{
	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);

	string temp;
	char* c_string_buffer;
	while (myThreadArgs->myDispenser->getNumPacketsToSend())
	{
		temp = myThreadArgs->myDispenser->getPacket();
		myThreadArgs->myUDP->send((char*)temp.c_str());
		//PRINT
		/*
		pthread_mutex_lock(&print_lock);
		cout << "Thread #: " << myThreadArgs->id;
		cout << " Got Packet #: " << get_sequence_number(temp) << endl;
		cout << "Contains:" << endl << temp << endl;
		pthread_mutex_unlock(&print_lock);
		*/

	}
}
int get_sequence_number(string packet)
{
	int higher = (int)(unsigned char)packet[1];
	int lower = (int)(unsigned char)packet[0];
	int output = (higher << 8) | lower;
	return output;
}
char* readFileBytes(const char* name, int& length)
{
	ifstream fl(name);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	char* ret = new char[len];
	fl.seekg(0, ios::beg);
	fl.read(ret, len);
	fl.close();
	length = len;
	return ret;
}

void read_from_file(const char* file_name, int packet_size, int sequencing_bytes, vector<string>& output)
{
	int length;
	char* file_bytes = readFileBytes(file_name, length);
	int count = 0;
	char* working;
	unsigned char* bytes;
	int bytes_returned;
	int data_packet_size = packet_size - (sequencing_bytes + 1);
	for (int i = 0; i < length; i++)
	{
		if (!(i % data_packet_size))
		{
			if (i)
			{
				working[packet_size - 1] = '\0';
				string temp(working);
				output.push_back(temp);
				free(working);
			}

			working = new char[packet_size];
			//put sequence bytes
			int_to_bytes(count, &bytes, bytes_returned);
			for (int j = 0; j < sequencing_bytes; j++)
			{
				if (j < bytes_returned)
				{
					working[j] = bytes[j];
				}
				else working[j] = 0;
			}
			if (i) free(bytes);
			count++;

		}
		working[(i % data_packet_size) + sequencing_bytes] = file_bytes[i];

	}
	free(file_bytes);
	//TODO: CONER CASE LAST PACKET PARTIALLY FULL
	return;
}

int main(int argc, char** argv)
{

	pthread_mutex_init(&print_lock, NULL); //for debug

	//**************** CLI ***************************
	if (argc != 5)
	{
		cout << endl << "Invalid Input" << endl;
		return 0;
	}


	char* Client_IP_Address = argv[1];
	char* Host_Port_Num = argv[2];
	char* Client_Port_Num = argv[3];
	char* File_Path = argv[4];



	//**************** Initialize Objects ***************************

	UDP* sessionUDP = new UDP(Client_IP_Address, Host_Port_Num, Client_Port_Num);
	sessionUDP->setPacketSize(PACKET_SIZE);
	vector<string> raw_data;
	read_from_file(File_Path, PACKET_SIZE, SEQUENCE_BYTE_NUM, raw_data);
	PacketDispenser* sessionPacketDispenser = new PacketDispenser(raw_data);


	//**************** Initialize Send Threads ***************************
	pthread_t* temp_p_thread;
	ThreadArgs* threadArgsTemp;
	int rc;
	vector<ThreadArgs*> sending_threads;
	for (int i = 0; i < NUM_SENDING_THREADS; i++)
	{


		temp_p_thread = new pthread_t;
		threadArgsTemp = new ThreadArgs(temp_p_thread, i, sessionUDP,
		                                sessionPacketDispenser);
		sending_threads.push_back(threadArgsTemp);
		rc = pthread_create(threadArgsTemp->self, NULL, sender_thread_function,
		                    (void*)threadArgsTemp);
	}

	//**************** Kill Send Threads ***************************


	for (auto thread : sending_threads)
	{
		pthread_join(*thread->self, NULL);
	}




}
