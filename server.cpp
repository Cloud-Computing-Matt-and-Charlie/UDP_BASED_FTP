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
#include<packet_dispenser.h>
#include<UDP.h>

void sender_thread(UDP* my_udp, PacketDispenser* my_packet_dispenser)
{
	while (my_packet_dispenser->getNumPacketsToSend())
	{
		my_udp->send(my_packet_dispenser->getPacket().c_str());
	}
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

int main(int argc, char** argv)
{


	if (argc != 4)
	{
		cout << endl << "Invalid Input" << endl;
		return 0;
	}

	char* Client_IP_Address = argv[1];
	char* Client_Port_Num = argv[2];
	char* File_Path = argv[3];






