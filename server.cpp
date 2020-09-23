/******************************************************//*
Creators: Matthew Pisini, Charles Bennett
Date: 9/19/20

Description:
Inputs:
1. IP address of the client
2. Port number of the host
3. Port number of client
4. Path to the directory of the file to be sent to client.

*//******************************************************/


#include "UDP.h"
#include "packet_dispenser.h"
#include<iostream>
#include<iostream>
#include<cmath>
#include<vector>
#include <fstream>
#include <streambuf>
#define SEQUENCE_BYTE_NUM 4
#define NUM_SENDING_THREADS 2
#define NUM_RECIEVING_THREADS 1
#define ACK_RESEND_THRESHOLD 3
#define PRINT 0
#define PRINT_R 1
#define NULL_TERMINATOR 0
#define DATA_SEGS 6
#define MAX_CON_SEG 2
<<<<<<< HEAD
int PACKET_SIZE = 1500;
int RECV_PACKET_SIZE = 1000; 
=======
int PACKET_SIZE = 256;
int RECV_PACKET_SIZE = 4; 
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e


pthread_mutex_t print_lock;
void* sender_thread_function(void* input_param);
int get_sequence_number(string packet);
char* readFileBytes(const char* name, int& length);
void read_from_file(ifstream& input_file, int packet_size,
                    int sequencing_bytes, vector<vector<char>>& output,
                    int total_bytes);
int get_file_length(const char* file_name);
char* vector_to_cstring(vector<char> input);
vector<char> cstring_to_vector(char* input, int size);
void* reciever_thread_function(void* input_param);
void launch_threads(void* input_param);
void* launch_segement_threads(void* input_param);
void launch_threads(PacketDispenser* sessionPacketDispenser, vector<UDP*>& sessionUDPs,
                    int data_seg, int offset);
void add_offset(vector<char>& input, int offset);
unsigned long vector_bytes_to_int(vector<char> input, unsigned long beign, unsigned long end); 
bool in_between(unsigned long i, unsigned long a, unsigned long b);


struct ThreadArgs
{
	ThreadArgs(pthread_t* self_in, int id_in, UDP* myUDP_in,
	           PacketDispenser* myDispenser_in, int data_seg_in,
	           int offset_in) :
		id{id_in}, myUDP{myUDP_in}, myDispenser{myDispenser_in}, self{self_in},
		data_seg{data_seg_in}, offset{offset_in}
	{};
	pthread_t* self;
	int id;
	int data_seg;
	int offset;
	UDP* myUDP;
	PacketDispenser* myDispenser;

};


struct SegArgs
{
	SegArgs(vector<UDP*> myUDPs_in, PacketDispenser* myDispenser_in, int data_seg_in,
	        pthread_t* self_in, int offset_in):
		myUDPs{myUDPs_in}, myDispenser{myDispenser_in}, data_seg{data_seg_in},
		self{self_in}, offset{offset_in}
	{};
	vector<UDP*> myUDPs;
	PacketDispenser* myDispenser;
	int data_seg;
	int offset;
	pthread_t* self;


};

class ListenThread
{
	private: 
		pthread_mutex_t info_lock; 
		pthread_t* self; 
		vector<PacketDispenser*> myDispensers; 
		UDP* myUDP; 
		string id; 
		vector<unsigned long> bandwidths; 
		vector<int> lengths; 
	public: 
		
		ListenThread(UDP* myUDP_in, vector<PacketDispenser*> myDispensers_in, pthread_t* self_in, 
		vector<int> lengths_in):
		myUDP{myUDP_in}, myDispensers{myDispensers_in}, self{self_in}, lengths{lengths_in}
		{
			this->id = "Listener Thread"; 
			pthread_mutex_init(&this->info_lock, NULL); 

		}
		ListenThread(UDP* myUDP_in, pthread_t* self_in) : 
		myUDP{myUDP_in}, self{self_in}
		{
			this->id = "Listener Thread"; 
			pthread_mutex_init(&this->info_lock, NULL); 

		}
		void getAckLocks()
		{
			pthread_mutex_lock(&this->info_lock); 
			for (auto disp : this->myDispensers)
			{
				disp->getAckLock();
			}
			pthread_mutex_unlock(&this->info_lock); 
			return; 
<<<<<<< HEAD
		}
		void releaseAckLocks()
		{
			pthread_mutex_lock(&this->info_lock); 
			for (auto disp : this->myDispensers)
			{
				disp->releaseAckLock();
			}
			pthread_mutex_unlock(&this->info_lock); 
			return; 
		}

		void addPacketDispenser(PacketDispenser* PacketDispenser_in, int length)
		{
			if (this->myDispensers.size() == 0) 
				pthread_mutex_unlock(&this->info_lock); 
			pthread_mutex_lock(&this->info_lock); 
			this->myDispensers.push_back(PacketDispenser_in); 
			this->lengths.push_back(length); 
			pthread_mutex_unlock(&this->info_lock); 
			return; 
		}
		int getGlobalAllAcksRecieved()
		{
			pthread_mutex_lock(&this->info_lock); 
			int temp = 1; 
			PacketDispenser* disp; 
			for (int i = 0; i<this->myDispensers.size(); i++)
			{
				disp = this->myDispensers[i]; 
				temp &= disp->getAllAcksRecieved(); 
				cout << "temp = " << temp << endl;
				if (disp->getAllAcksRecieved())
				{
					pthread_mutex_lock(&this->info_lock); 
					this->bandwidths.push_back(disp->getBandwidth()); 
					this->myDispensers.erase(this->myDispensers.begin()+i); 
					delete disp; 
					this->lengths.erase(this->lengths.begin()+i); 
					i = i-1; 
					pthread_mutex_unlock(&this->info_lock); 

				}
			}
			pthread_mutex_unlock(&this->info_lock); 
			return temp; 
		}
		void print_exit()
		{
			unsigned long runsum = 0; 
			for (auto entry : this->bandwidths) runsum += entry; 
			cout<<"Bandwidth is "<<runsum<<endl; 
		}
		void globalPutAcks(vector<char> packet_in)
		{
			cout<<"Enter global put acks"<<endl;
			pthread_mutex_lock(&this->info_lock); 
			this->getAckLocks(); 
			int count = 0; 
			unsigned long ack_index; 
			int range_loc = 0; 
			while (count < packet_in.size())
			{
				ack_index = vector_bytes_to_int(packet_in, count, count+SEQUENCE_BYTE_NUM - 1); 
				count+= SEQUENCE_BYTE_NUM; 
				range_loc = 0; 
				if (PRINT_R) cout<<"recieved ack # "<<ack_index<<endl;
				for (int i = 0; i<this->myDispensers.size(); i++)
				{
					if (in_between(ack_index, range_loc, range_loc+this->lengths[i])) 
					{
						if(PRINT_R) cout<<ack_index<<" Belongs inside the "<<i<<"th Segement"<<endl;
						this->myDispensers[i]->putAck(ack_index - range_loc); 
						break; 
					}
					range_loc += this->lengths[i]; 
				}
			}
			this->releaseAckLocks(); 
			pthread_mutex_unlock(&this->info_lock); 
			return; 
		}
		void* doListen()
		{
			cout << "this->myDispensers.size() = " << this->myDispensers.size() << endl;
			if (this->myDispensers.size() == 0)
				pthread_mutex_lock(&this->info_lock); 
			vector<char> buffer; 
			int working; 
			int top; 
			int bytes_size; 
			char* temp; 
			while (!(this->getGlobalAllAcksRecieved()))
			{
				temp = this->myUDP->recieve(bytes_size); 
				buffer = cstring_to_vector(temp, bytes_size);
				//buffer = cstring_to_vector(this->myUDP->recieve(bytes_size), bytes_size);
				// int j = 0;
				// for(int i = 0; i < SEQUENCE_BYTE_NUM; i+=2)
				// {
				// 	unsigned char f[2] = {temp[i],temp[i+1]};
				// 	j = bytes_to_int(f,2);
				// 	// cout << j << endl;
				// }
				cout<<"recieved packet" << endl;
				cout << buffer.size() << endl;
				this->globalPutAcks(buffer); 
				cout <<"back from putacks" << endl;
			}
			this->print_exit(); 

			pthread_exit(NULL);
			

		}
		static void* threadLauncher(void* input)
		{
			cout<<"ENTER DO LISTEN"<<endl;
			return ((ListenThread*)(input))->doListen(); 
		}
=======
		}
		void releaseAckLocks()
		{
			pthread_mutex_lock(&this->info_lock); 
			for (auto disp : this->myDispensers)
			{
				disp->releaseAckLock();
			}
			pthread_mutex_unlock(&this->info_lock); 
			return; 
		}

		void addPacketDispenser(PacketDispenser* PacketDispenser_in, int length)
		{
			if (this->myDispensers.size() == 0) 
				pthread_mutex_unlock(&this->info_lock); 
			pthread_mutex_lock(&this->info_lock); 
			this->myDispensers.push_back(PacketDispenser_in); 
			this->lengths.push_back(length); 
			pthread_mutex_unlock(&this->info_lock); 
			return; 
		}
		int getGlobalAllAcksRecieved()
		{
			pthread_mutex_lock(&this->info_lock); 
			int temp = 1; 
			PacketDispenser* disp; 
			for (int i = 0; i<this->myDispensers.size(); i++)
			{
				disp = this->myDispensers[i]; 
				temp &= disp->getAllAcksRecieved(); 
				if (disp->getAllAcksRecieved())
				{
					pthread_mutex_lock(&this->info_lock); 
					this->bandwidths.push_back(disp->getBandwidth()); 
					this->myDispensers.erase(this->myDispensers.begin()+i); 
					delete disp; 
					this->lengths.erase(this->lengths.begin()+i); 
					i = i-1; 
					pthread_mutex_unlock(&this->info_lock); 

				}
			}
			pthread_mutex_unlock(&this->info_lock); 
			return temp; 
		}
		void print_exit()
		{
			unsigned long runsum = 0; 
			for (auto entry : this->bandwidths) runsum += entry; 
			cout<<"Bandwidth is "<<runsum<<endl; 
		}
		void globalPutAcks(vector<char> packet_in)
		{
			cout<<"Enter global put acks"<<endl;
			pthread_mutex_lock(&this->info_lock); 
			this->getAckLocks(); 
			int count = 0; 
			unsigned long ack_index; 
			int range_loc = 0; 
			while (count < packet_in.size())
			{
				ack_index = vector_bytes_to_int(packet_in, count, count+SEQUENCE_BYTE_NUM - 1); 
				count+= SEQUENCE_BYTE_NUM; 
				range_loc = 0; 
				if (PRINT_R) cout<<"recieved ack # "<<ack_index<<endl;
				for (int i = 0; i<this->myDispensers.size(); i++)
				{
					if (in_between(ack_index, range_loc, range_loc+this->lengths[i])) 
					{
						if(PRINT_R) cout<<ack_index<<" Belongs inside the "<<i<<"th Segement"<<endl;
						this->myDispensers[i]->putAck(ack_index - range_loc); 
						break; 
					}
					range_loc += this->lengths[i]; 
				}
			}
			this->releaseAckLocks(); 
			pthread_mutex_unlock(&this->info_lock); 
			return; 
		}
		void* doListen()
		{
			
			if (this->myDispensers.size() == 0)
				pthread_mutex_lock(&this->info_lock); 
			vector<char> buffer; 
			int working; 
			int top; 
			int bytes_size; 
			char* temp; 
			while (!(this->getGlobalAllAcksRecieved()))
			{
				temp = this->myUDP->recieve(bytes_size); 
				buffer = cstring_to_vector(temp, bytes_size);
				//buffer = cstring_to_vector(this->myUDP->recieve(bytes_size), bytes_size);
				cout<<"recieved "<<vector_bytes_to_int(buffer, 0, 1)<<endl;
				this->globalPutAcks(buffer); 
			}
			this->print_exit(); 

			pthread_exit(NULL);
			

		}
		static void* threadLauncher(void* input)
		{
			cout<<"ENTER DO LISTEN"<<endl;
			return ((ListenThread*)(input))->doListen(); 
		}
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e

};

void* launch_segement_threads(void* input_param)
{
	SegArgs* mySegArgs = (SegArgs*)(input_param);
	PacketDispenser* sessionPacketDispenser = mySegArgs->myDispenser;
	vector<UDP*> sessionUDPs = mySegArgs->myUDPs;
	int data_seg = mySegArgs->data_seg;
	int offset = mySegArgs->offset;


//**************** Initialize Send Threads ***************************
	pthread_t* temp_p_thread;
	ThreadArgs* threadArgsTemp;
	int rc;
	vector<ThreadArgs*> sending_threads;
	for (int i = 0; i < NUM_SENDING_THREADS; i++)
	{

		temp_p_thread = new pthread_t;
		threadArgsTemp = new ThreadArgs(temp_p_thread, i, sessionUDPs[i],
		                                sessionPacketDispenser, data_seg,
		                                offset);
		sending_threads.push_back(threadArgsTemp);
		rc = pthread_create(threadArgsTemp->self, NULL, sender_thread_function,
		                    (void*)threadArgsTemp);
	}


	for (auto thread : sending_threads)
	{
		pthread_join(*thread->self, NULL);
	}
<<<<<<< HEAD

	pthread_exit(NULL);

}
=======
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e

	pthread_exit(NULL);

<<<<<<< HEAD
=======
}


>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e
void* sender_thread_function(void* input_param)
{
	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);

	vector<char> temp;
	char* c_string_buffer;
	while (!myThreadArgs->myDispenser->getAllAcksRecieved())
	{

		temp = myThreadArgs->myDispenser->getPacket();
		add_offset(temp, myThreadArgs->offset);

		if (!temp.empty())
		{

			myThreadArgs->myUDP->send(vector_to_cstring(temp));
			int num_temp = (int)(((unsigned char)(temp[0])) << 8);
			num_temp |= ((unsigned char)temp[1]);

			if (PRINT) cout << "Thread #: " << myThreadArgs->id;
			if (PRINT) cout << " Packet #: " << num_temp << endl;
			if (PRINT) cout << "Current Bandwidth " << myThreadArgs->myDispenser->getBandwidth() << endl;
			if (PRINT) cout << "Time since last packet " <<
				                myThreadArgs->myDispenser->getTimeSinceLastPacket() << endl;

		}
		if (PRINT) cout << "Current Bandwidth " << myThreadArgs->myDispenser->getBandwidth() << endl;
		if (myThreadArgs->id == 0)
		{
			myThreadArgs->myDispenser->resendOnTheshold(ACK_RESEND_THRESHOLD);
		}
	}
	cout << endl << endl << endl << "THREAD # " << myThreadArgs->id << " EXIT" << endl << endl;
	pthread_exit(NULL);
}


void read_from_file(ifstream& input_file, int packet_size, int sequencing_bytes,
                    vector<vector<char>>& output, int total_bytes)
{
	cout << "Reading " << total_bytes << " bytes from file" << endl;
	int length;
	if (!length) cout << " BAD FILE ERROR" << endl;
	int count = 0;
	vector<char> working(packet_size);
	unsigned char* bytes;
	int bytes_returned;
	int null_terminator = 0;
	int data_packet_size = packet_size - (sequencing_bytes + NULL_TERMINATOR);
	int remainder;
	char* cstring_buff = new char[packet_size];
	while (((count)*data_packet_size) <= total_bytes)
	{
		remainder = total_bytes - (count * data_packet_size);
		int_to_bytes(count, &bytes, bytes_returned);
		for (int j = sequencing_bytes - 1; j >= 0; j--)
		{
			if ((sequencing_bytes  - j) <= bytes_returned)
			{
				cstring_buff[j] = bytes[(bytes_returned - 1) + (j + 1 - sequencing_bytes)];
			}
			else cstring_buff[j] = 0;
		}
		if (count) free(bytes);
		if (remainder < data_packet_size)
			input_file.read((cstring_buff + sequencing_bytes), remainder);
		else
			input_file.read((cstring_buff + sequencing_bytes), data_packet_size);
		if (NULL_TERMINATOR)
		{
			cstring_buff[packet_size - 1] = '\0';
		}
		output.push_back(cstring_to_vector(cstring_buff, packet_size));
		count++;
	}
	free(cstring_buff);
	cout << "Read " << total_bytes << " Bytes ";
	cout << "Into " << output.size() << " Packets " << endl;

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




	int UDP_needed = NUM_SENDING_THREADS;
	vector<UDP*> sessionUDPs(UDP_needed);

	sessionUDPs[0] = new UDP(Client_IP_Address, Host_Port_Num, Client_Port_Num);
	char* temp_char = "000";
	for (int i = 0; i < UDP_needed; i++)
	{
		if (i != 0) sessionUDPs[i] = new UDP(Client_IP_Address, temp_char, Client_Port_Num);
<<<<<<< HEAD
		sessionUDPs[i]->setPacketSize(RECV_PACKET_SIZE);
		sessionUDPs[i]->setSendPacketSize(PACKET_SIZE); 
	}

	ifstream fl(File_Path, ios::binary | ios::in);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	int file_length = (int)len;
	cout << "file length is " << file_length << " bytes" << endl;
	fl.seekg(0, ios::beg);
	vector<int> seg_lengths;
	int seg_length = file_length / DATA_SEGS;
	for (int i = 0; i < DATA_SEGS; i++)
	{
		if (i == (DATA_SEGS - 1))
			seg_lengths.push_back(seg_length + (file_length % DATA_SEGS));
		else
			seg_lengths.push_back(seg_length);
	}
	vector<vector<vector<char>>> raw_datas(DATA_SEGS);
	pthread_t* temp_p_thread;
	SegArgs* tempSegArgs;
	int rc; 
	int offset = 0;
	int joined = 0;
	vector<pthread_t*> seg_threads;
	pthread_t listen_pid; 
	ListenThread* sessionListenThread = new ListenThread(sessionUDPs[0], &listen_pid); 
	rc = pthread_create(&listen_pid, NULL, sessionListenThread->threadLauncher, ((void*)sessionListenThread)); 
	for (int i = 0; i < DATA_SEGS; i++)
	{
		temp_p_thread = new pthread_t;
		read_from_file(fl, PACKET_SIZE, SEQUENCE_BYTE_NUM, raw_datas[i], seg_lengths[i]);
		cout << "legnth of data " << i << " is " << raw_datas[i].size() << endl;
		PacketDispenser* sessionPacketDispenser = new PacketDispenser(raw_datas[i]);
		sessionListenThread->addPacketDispenser(sessionPacketDispenser, raw_datas[i].size()); 
		sessionPacketDispenser->setMaxBandwidth(10);
		if (i >= MAX_CON_SEG)
		{
			for (auto entry : seg_threads)
			{
				pthread_join(*entry, NULL);
				joined++;
			}
		}
		tempSegArgs = new SegArgs(sessionUDPs, sessionPacketDispenser, i,
		                          temp_p_thread, offset);
		rc = pthread_create(tempSegArgs->self, NULL, launch_segement_threads,
		                    ((void*)tempSegArgs));
		seg_threads.push_back(tempSegArgs->self);
		offset += raw_datas[i].size();
		raw_datas[i].clear(); 
		raw_datas[i].shrink_to_fit(); 
		
	}
	for (int i = joined; i < DATA_SEGS; i++)
	{
		pthread_join(*seg_threads[i], NULL);
	}

}

/*

cout << endl << endl << endl;
cout << "***************************" << endl;
cout << "Sucess!" << endl;
cout << raw_data.size() << " Packets sent each with " << PACKET_SIZE << " bytes " << endl;
cout << "For a total of " << raw_data.size()*PACKET_SIZE << " bytes " << endl;
cout << "All sent in just " << sessionPacketDispenser->getTotalTime() << " seconds" << endl;
double bw = raw_data.size() * PACKET_SIZE;
bw = (bw / ((double)(sessionPacketDispenser->getTotalTime())));
cout << " Bandwidth = " << bw << " bytes per second" << endl;
cout << "***************************" << endl;

*/

bool in_between(unsigned long i, unsigned long a, unsigned long b)
{
	if (i>=a)
	{
		if (i<b) return true; 
	}
	return false; 
}

void add_offset(vector<char>& input, int offset)
{

	int start = bytes_to_int((unsigned char*)vector_to_cstring(input), SEQUENCE_BYTE_NUM);
	start += offset;
	unsigned char* bytes;
	int bytes_returned;

	int_to_bytes(start, &bytes, bytes_returned);
	for (int j = SEQUENCE_BYTE_NUM - 1; j >= 0; j--)
	{
		if ((SEQUENCE_BYTE_NUM  - j) <= bytes_returned)
		{
			input[j] = bytes[(bytes_returned - 1) + (j + 1 - SEQUENCE_BYTE_NUM)];
		}
		else input[j] = 0;
	}
	if (bytes_returned) free(bytes);
	return;
}

unsigned long vector_bytes_to_int(vector<char> input, unsigned long start, unsigned long end)
{
	//MSB ... LSB 
	unsigned long output = 0; 
	for (int i = end; i>=start; i--)
	{
		output |= ((unsigned long)((unsigned char)(input[i]) << (8*i))); 
	}
	return output; 
}



/*
void launch_threads(PacketDispenser* sessionPacketDispenser, vector<UDP*>& sessionUDPs,
                    int data_seg, int offset)
{

=======
		sessionUDPs[i]->setPacketSize(PACKET_SIZE);
		sessionUDPs[i]->setSendPacketSize(RECV_PACKET_SIZE); 
	}

	ifstream fl(File_Path, ios::binary | ios::in);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	int file_length = (int)len;
	cout << "file length is " << file_length << " bytes" << endl;
	fl.seekg(0, ios::beg);
	vector<int> seg_lengths;
	int seg_length = file_length / DATA_SEGS;
	for (int i = 0; i < DATA_SEGS; i++)
	{
		if (i == (DATA_SEGS - 1))
			seg_lengths.push_back(seg_length + (file_length % DATA_SEGS));
		else
			seg_lengths.push_back(seg_length);
	}
	vector<vector<vector<char>>> raw_datas(DATA_SEGS);
	pthread_t* temp_p_thread;
	SegArgs* tempSegArgs;
	int rc; 
	int offset = 0;
	int joined = 0;
	vector<pthread_t*> seg_threads;
	pthread_t listen_pid; 
	ListenThread* sessionListenThread = new ListenThread(sessionUDPs[0], &listen_pid); 
	rc = pthread_create(&listen_pid, NULL, sessionListenThread->threadLauncher, ((void*)sessionListenThread)); 
	for (int i = 0; i < DATA_SEGS; i++)
	{
		temp_p_thread = new pthread_t;
		read_from_file(fl, PACKET_SIZE, SEQUENCE_BYTE_NUM, raw_datas[i], seg_lengths[i]);
		cout << "legnth of data " << i << " is " << raw_datas[i].size() << endl;
		PacketDispenser* sessionPacketDispenser = new PacketDispenser(raw_datas[i]);
		sessionListenThread->addPacketDispenser(sessionPacketDispenser, raw_datas[i].size()); 
		sessionPacketDispenser->setMaxBandwidth(10);
		if (i >= MAX_CON_SEG)
		{
			for (auto entry : seg_threads)
			{
				pthread_join(*entry, NULL);
				joined++;
			}
		}
		tempSegArgs = new SegArgs(sessionUDPs, sessionPacketDispenser, i,
		                          temp_p_thread, offset);
		rc = pthread_create(tempSegArgs->self, NULL, launch_segement_threads,
		                    ((void*)tempSegArgs));
		seg_threads.push_back(tempSegArgs->self);
		offset += raw_datas[i].size();
		raw_data[i].clear(); 
		raw_data[i].shrink_to_fit(); 
		
	}
	for (int i = joined; i < DATA_SEGS; i++)
	{
		pthread_join(*seg_threads[i], NULL);
	}

}

/*

cout << endl << endl << endl;
cout << "***************************" << endl;
cout << "Sucess!" << endl;
cout << raw_data.size() << " Packets sent each with " << PACKET_SIZE << " bytes " << endl;
cout << "For a total of " << raw_data.size()*PACKET_SIZE << " bytes " << endl;
cout << "All sent in just " << sessionPacketDispenser->getTotalTime() << " seconds" << endl;
double bw = raw_data.size() * PACKET_SIZE;
bw = (bw / ((double)(sessionPacketDispenser->getTotalTime())));
cout << " Bandwidth = " << bw << " bytes per second" << endl;
cout << "***************************" << endl;

*/

bool in_between(unsigned long i, unsigned long a, unsigned long b)
{
	if (i>=a)
	{
		if (i<b) return true; 
	}
	return false; 
}

void add_offset(vector<char>& input, int offset)
{

	int start = bytes_to_int((unsigned char*)vector_to_cstring(input), SEQUENCE_BYTE_NUM);
	start += offset;
	unsigned char* bytes;
	int bytes_returned;

	int_to_bytes(start, &bytes, bytes_returned);
	for (int j = SEQUENCE_BYTE_NUM - 1; j >= 0; j--)
	{
		if ((SEQUENCE_BYTE_NUM  - j) <= bytes_returned)
		{
			input[j] = bytes[(bytes_returned - 1) + (j + 1 - SEQUENCE_BYTE_NUM)];
		}
		else input[j] = 0;
	}
	if (bytes_returned) free(bytes);
	return;
}

unsigned long vector_bytes_to_int(vector<char> input, unsigned long start, unsigned long end)
{
	//MSB ... LSB 
	unsigned long output = 0; 
	for (int i = end; i>=start; i--)
	{
		output |= ((unsigned long)((unsigned char)(input[i]) << (8*i))); 
	}
	return output; 
}



/*
void launch_threads(PacketDispenser* sessionPacketDispenser, vector<UDP*>& sessionUDPs,
                    int data_seg, int offset)
{

>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e
//**************** Initialize Send Threads ***************************
	pthread_t* temp_p_thread;
	ThreadArgs* threadArgsTemp;
	int rc;
	vector<ThreadArgs*> sending_threads;
	for (int i = 0; i < NUM_SENDING_THREADS; i++)
	{

		temp_p_thread = new pthread_t;
		threadArgsTemp = new ThreadArgs(temp_p_thread, i, sessionUDPs[i],
		                                sessionPacketDispenser, data_seg, offset);
		sending_threads.push_back(threadArgsTemp);
		rc = pthread_create(threadArgsTemp->self, NULL, sender_thread_function,
		                    (void*)threadArgsTemp);
	}
//**************** Initialize Recieve Threads ***************************
	vector<ThreadArgs*> recieving_threads;
	for (int i = NUM_SENDING_THREADS; i < NUM_RECIEVING_THREADS + NUM_SENDING_THREADS; i++)
	{

		temp_p_thread = new pthread_t;
		threadArgsTemp = new ThreadArgs(temp_p_thread, i, sessionUDPs[0],
		                                sessionPacketDispenser, data_seg, offset);
		recieving_threads.push_back(threadArgsTemp);
		rc = pthread_create(threadArgsTemp->self, NULL, reciever_thread_function,
		                    (void*)threadArgsTemp);
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
	
<<<<<<< HEAD

}

*/
int get_sequence_number(string packet)
{
	int higher = (int)(unsigned char)packet[1];
	int lower = (int)(unsigned char)packet[0];
	int output = (higher << 8) | lower;
	return output;
}

char* readFileBytes(const char* name, int& length)
{
	cout << "enter read file" << endl;
	ifstream fl(name, ios::binary | ios::in);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	char* ret = new char[len];
	fl.seekg(0, ios::beg);
	fl.read(ret, len);
	fl.close();
	length = len;
	cout << "done reading" << endl;
	return ret;
}
int get_file_length(const char* name)
{
	ifstream fl(name);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	fl.close();
	return (int)len;
}


void* reciever_thread_function(void* input_param)
{
	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);
	vector<char> buffer;
	int working;
	int top;
	int bytes_size;

	while (!myThreadArgs->myDispenser->getAllAcksRecieved())
	{
		//todo think about deadlock on final packet


		buffer = cstring_to_vector(myThreadArgs->myUDP->recieve(bytes_size), bytes_size);
		top = 1;
		myThreadArgs->myDispenser->getAckLock();
		for (auto entry : buffer)
		{
			if (!top)
			{
				working |= ((unsigned char)(entry));
				myThreadArgs->myDispenser->putAck(working - myThreadArgs->offset);
				working = 0;

			}
			top = !top;
			working = (((unsigned char)entry) << 8);
		}
		myThreadArgs->myDispenser->releaseAckLock();
	}
	pthread_exit(NULL);
}
=======
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e

}

<<<<<<< HEAD
/*
void read_from_file(const char* file_name, int packet_size, int sequencing_bytes, vector<vector<char>>& output)
{
	int length;
	char* file_bytes = readFileBytes(file_name, length);
	if (!length) cout << " BAD FILE ERROR" << endl;
	int count = 0;
	vector<char> working(packet_size);
	unsigned char* bytes;
	int bytes_returned;
	int null_terminator = 0;
	int data_packet_size = packet_size - (sequencing_bytes + null_terminator);
	for (int i = 0; i < length; i++)
	{
		if (!(i % data_packet_size))
		{
			if (i)
			{
				if (null_terminator) working[packet_size - 1] = '\0';
				output.push_back(working);

			}
=======
*/
int get_sequence_number(string packet)
{
	int higher = (int)(unsigned char)packet[1];
	int lower = (int)(unsigned char)packet[0];
	int output = (higher << 8) | lower;
	return output;
}

char* readFileBytes(const char* name, int& length)
{
	cout << "enter read file" << endl;
	ifstream fl(name, ios::binary | ios::in);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	char* ret = new char[len];
	fl.seekg(0, ios::beg);
	fl.read(ret, len);
	fl.close();
	length = len;
	cout << "done reading" << endl;
	return ret;
}
int get_file_length(const char* name)
{
	ifstream fl(name);
	fl.seekg( 0, ios::end );
	size_t len = fl.tellg();
	fl.close();
	return (int)len;
}
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e

			int_to_bytes(count, &bytes, bytes_returned);

<<<<<<< HEAD
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
=======
void* reciever_thread_function(void* input_param)
{
	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);
	vector<char> buffer;
	int working;
	int top;
	int bytes_size;

	while (!myThreadArgs->myDispenser->getAllAcksRecieved())
	{
		//todo think about deadlock on final packet
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e

		}
		working[(i % data_packet_size) + sequencing_bytes] = file_bytes[i];

<<<<<<< HEAD
	}
	free(file_bytes);
	//TODO: CONER CASE LAST PACKET PARTIALLY FULL
	return;
}
=======
		buffer = cstring_to_vector(myThreadArgs->myUDP->recieve(bytes_size), bytes_size);
		top = 1;
		myThreadArgs->myDispenser->getAckLock();
		for (auto entry : buffer)
		{
			if (!top)
			{
				working |= ((unsigned char)(entry));
				myThreadArgs->myDispenser->putAck(working - myThreadArgs->offset);
				working = 0;

			}
			top = !top;
			working = (((unsigned char)entry) << 8);
		}
		myThreadArgs->myDispenser->releaseAckLock();
	}
	pthread_exit(NULL);
}


/*
void read_from_file(const char* file_name, int packet_size, int sequencing_bytes, vector<vector<char>>& output)
{
	int length;
	char* file_bytes = readFileBytes(file_name, length);
	if (!length) cout << " BAD FILE ERROR" << endl;
	int count = 0;
	vector<char> working(packet_size);
	unsigned char* bytes;
	int bytes_returned;
	int null_terminator = 0;
	int data_packet_size = packet_size - (sequencing_bytes + null_terminator);
	for (int i = 0; i < length; i++)
	{
		if (!(i % data_packet_size))
		{
			if (i)
			{
				if (null_terminator) working[packet_size - 1] = '\0';
				output.push_back(working);

			}

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
>>>>>>> 96035b04a8647991e44b10325cbf2d29e20f376e
*/




