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
#define NUM_SENDING_THREADS 2
#define NUM_RECIEVING_THREADS 1
#define ACK_RESEND_THRESHOLD 3

int PACKET_SIZE = 16;
pthread_mutex_t print_lock;
void* sender_thread_function(void* input_param);
int get_sequence_number(string packet);
char* readFileBytes(const char* name, int& length);
void read_from_file(const char* file_name, int packet_size,
                    int sequencing_bytes, vector<vector<char>>& output);
char* vector_to_cstring(vector<char> input);
vector<char> cstring_to_vector(char* input, int size);
void* reciever_thread_functin(void* input_param);



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

	vector<char> temp;
	char* c_string_buffer;
	while (myThreadArgs->myDispenser->getNumPacketsToSend())
	{
		temp = myThreadArgs->myDispenser->getPacket();
		myThreadArgs->myUDP->send(vector_to_cstring(temp));
		int num_temp = (int)(((unsigned char)(temp[0])) << 8);
		num_temp |= ((unsigned char)temp[1]);

		cout << "Thread #: " << myThreadArgs->id;
		cout << " Packet #: " << num_temp << endl;
		if (myThreadArgs->id == 0)
		{
			myThreadArgs->myDispenser->resendOnTheshold(ACK_RESEND_THRESHOLD);
		}

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



void* reciever_thread_function(void* input_param)
{
	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);
	vector<char> buffer;
	int working;
	int top;
	int bytes_size;

	while (myThreadArgs->myDispenser->getNumPacketsToSend() &&
	        myThreadArgs->myDispenser->getAllAcksRecieved())
	{
		//todo think about deadlock on final packet

		buffer = cstring_to_vector(myThreadArgs->myUDP->recieve(bytes_size), bytes_size);
		top = 1;

		for (auto entry : buffer)
		{
			if (!top)
			{
				working |= ((unsigned char)(entry));
				cout << " now acking number " << std::dec << working << endl;
				myThreadArgs->myDispenser->putAck(working);
				working = 0;

			}
			top = !top;
			working = (((unsigned char)entry) << 8);
		}
		if (myThreadArgs->myDispenser->getNumPacketsToSend() < ACK_RESEND_THRESHOLD)
		{
			cout << "Adding Packets to Be Resent with ";
			cout << std::dec << myThreadArgs->myDispenser->getNumPacketsToSend();
			cout << " Packets Left in Queue" << endl;
			myThreadArgs->myDispenser->resendAll();
		}

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

void read_from_file(const char* file_name, int packet_size, int sequencing_bytes, vector<vector<char>>& output)
{
	int length;
	char* file_bytes = readFileBytes(file_name, length);
	int count = 0;
	char* working;
	unsigned char* bytes;
	int bytes_returned;
	int null_terminator = 1;
	int data_packet_size = packet_size - (sequencing_bytes + null_terminator);
	for (int i = 0; i < length; i++)
	{
		if (!(i % data_packet_size))
		{
			if (i)
			{
				if (null_terminator) working[packet_size - 1] = '\0';
				//string temp(working);

				//output.push_back(temp);
				output.push_back(cstring_to_vector(working, packet_size));
				//string test = string(output.back().begin() + 2, output.back().end());
				//cout << (test) << endl;
				free(working);
			}

			working = new char[packet_size];
			//put sequence bytes
			int_to_bytes(count, &bytes, bytes_returned);

			for (int j = sequencing_bytes - 1; j >= 0; j--)
			{
				if ((sequencing_bytes  - j) <= bytes_returned)
				{
					working[j] = bytes[(bytes_returned - 1) + (j + 1 - sequencing_bytes)];
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
	vector<vector<char>> raw_data;
	read_from_file(File_Path, PACKET_SIZE, SEQUENCE_BYTE_NUM, raw_data);
	PacketDispenser* sessionPacketDispenser = new PacketDispenser(raw_data);
	sessionPacketDispenser->setMaxBandwidth(1000000);


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
	//**************** Initialize Recieve Threads ***************************
	vector<ThreadArgs*> recieving_threads;
	for (int i = NUM_SENDING_THREADS; i < NUM_RECIEVING_THREADS + NUM_SENDING_THREADS; i++)
	{
		temp_p_thread = new pthread_t;
		threadArgsTemp = new ThreadArgs(temp_p_thread, i, sessionUDP,
		                                sessionPacketDispenser);
		recieving_threads.push_back(threadArgsTemp);
		rc = pthread_create(threadArgsTemp->self, NULL, reciever_thread_function,
		                    (void*)threadArgsTemp);
		cout << "Thread # " << threadArgsTemp->id << endl;
	}
	//**************** Kill Threads ***************************

	for (auto thread : recieving_threads)
	{
		pthread_join(*thread->self, NULL);
	}
	for (auto thread : sending_threads)
	{
		pthread_join(*thread->self, NULL);
	}








}
