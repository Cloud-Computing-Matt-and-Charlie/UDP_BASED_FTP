#include<iostream>
#include<cmath>
#include<vector>
#include <fstream>
#include <streambuf>
using namespace std;

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

int main()
{
	vector<string> output;
	read_from_file("test_file.txt", 16, 2, output);
	for (auto entry : output)
	{

		cout << "Packet #: " << endl;
		cout << std::hex << (int)(unsigned char)entry[1] << (int)(unsigned char)entry[0] << endl;
		cout << "Contains: ";
		cout << entry << endl;

	}
}