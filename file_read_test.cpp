#include<iostream>
#include<cmath>
#include<vector>
#include <fstream>
#include <streambuf>
#include "UDP.h"
#include "packet_dispenser.h"
using namespace std;
pthread_mutex_t print_lock;

/*
void int_to_bytes(unsigned int input, unsigned char** output, int& output_size)
{

	int bits = log2(input) + 1;
	if (!input)
	{
		output_size = 0;
		return;
	}
	int bytes = ceil((double)bits / 8);
	*output = new unsigned char[bytes];
	for ( int i = 0; i < bytes; i++)
	{
		(*output)[i] = (0xFF & input >> (8 * i));

	}
	output_size = bytes;
	return;

}
*/
/*

char* cstring_to_string(string input)
{
	char* output = new
}
*/

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
	int data_packet_size = packet_size - sequencing_bytes;
	for (int i = 0; i < length; i++)
	{
		if (!(i % data_packet_size))
		{
			if (i)
			{
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
int get_sequence_number(string packet)
{
	int higher = (int)(unsigned char)packet[1];
	int lower = (int)(unsigned char)packet[0];
	int output = (higher << 8) | lower;
	return output;
}

void* thread_function(void* input_param)
{
	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);

	string temp;
	char* c_string_buffer;
	while (myThreadArgs->myDispenser->getNumPacketsToSend())
	{
		temp = myThreadArgs->myDispenser->getPacket();
		myThreadArgs->myUDP->send((char*)temp.c_str());
		//myThreadArgs->myUDP->send(buffer);
		//PRINT
		pthread_mutex_lock(&print_lock);
		cout << "Thread #: " << myThreadArgs->id;
		cout << " Got Packet #: " << get_sequence_number(temp) << endl;
		cout << "Contains:" << endl << temp << endl;
		pthread_mutex_unlock(&print_lock);

	}
}

int main()
{


	pthread_t thread1;
	pthread_t thread2;

	pthread_mutex_init(&print_lock, NULL);
	pthread_mutex_unlock(&print_lock);
	UDP* my_udp = new UDP("127.0.0.1", "6234", "6235");
	my_udp->setPacketSize(16);


	vector<string> output;
	read_from_file("test_file.txt", 16, 2, output);
	PacketDispenser* my_packet_dispenser = new PacketDispenser(output);

	vector<ThreadArgs*> threads;
	int num_threads = 2;
	pthread_t* temp_p_thread;
	ThreadArgs* threadArgsTemp;

	int rc;
	for (int i = 0; i < num_threads; i++)
	{


		temp_p_thread = new pthread_t;
		threadArgsTemp = new ThreadArgs(temp_p_thread, i, my_udp, my_packet_dispenser);
		threads.push_back(threadArgsTemp);
		rc = pthread_create(threadArgsTemp->self, NULL, thread_function,
		                    (void*)threadArgsTemp);
	}

	//my_packet_dispenser->setMaxBandwidth(1000000000000);
	//int rc = pthread_create(&thread1, NULL, thread_function, (void*)my_packet_dispenser);
	//rc = pthread_create(&thread2, NULL, thread_function, (void*)my_packet_dispenser);
	//remember std::ref
	for (auto entry : output)
	{
		/*

		cout << "Packet #: " << endl;
		cout << std::hex << (int)(unsigned char)entry[1] << (int)(unsigned char)entry[0] << endl;
		cout << "Contains: ";
		cout << entry << endl;
		*/

	}



	//char working_buffer[17];
	//working_buffer[16] = '\n';
	char* working_buffer = "hello world whats up \0";
	string temp;

	/*
		while ( my_packet_dispenser->getNumPacketsToSend())
		{

			temp = my_packet_dispenser->getPacket();
			pthread_mutex_lock(&print_lock);
			cout << "Thread #: Main";
			cout << " Got Packet #: " << get_sequence_number(temp) << endl;
			cout << "Contains:" << endl << temp << endl;
			pthread_mutex_unlock(&print_lock);

		}
		*/
	for (auto thread : threads)
	{
		cout << "killing " << thread->id << endl;
		pthread_join(*thread->self, NULL);
	}

}