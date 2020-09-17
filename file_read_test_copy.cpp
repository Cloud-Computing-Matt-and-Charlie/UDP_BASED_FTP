#include<iostream>
using namespace std;

void int_to_bytes(unsigned int input, unsigned char** output, int& output_size)
{

	int bits = log2(input) + 1;
	int bytes = ceil((double)bits / 8);
	*output = new unsigned char[bytes];

	for ( int i = 0; i < bytes; i++)
	{
		(*output)[i] = (0xFF & input >> (8 * i));

	}
	output_size = bytes;
	return;

}

vector<char*> read_from_file(const char* file_name, int packet_size, int sequencing_bytes)
{
	int length;
	char* file_bytes = readFileBytes(file_name, length);
	int count = 0;
	char* working;
	unsigned char* bytes;
	int bytes_returned;
	int data_packet_size = packet_size - sequencing_bytes;
	vector<char*> output;
	for (int i = 0; i < length; i++)
	{
		if (!(i % data_packet_size))
		{
			if (i)
			{
				output.push_back(working);
			}
			free(working);
			working = new char[packet_size];
			//put sequence bytes
			int_to_bytes(count, &bytes, bytes_returned);
			for (j = 0; j < sequencing_bytes; j++)
			{
				if (j < bytes_returned)
				{
					working[j] = bytes[j];
				}
				else working[j] = 0;
			}
			free(bytes);
			count++;

		}
		working[(i % data_packet_size) + sequencing_bytes] = file_bytes[i];
	}
	free(file_bytes);
}

int main()
{
	vector<char*> output = read_from_file("text_file.txt", 16, 2);
	for (auto entry : output)
	{
		cout << entry << endl;
	}
}