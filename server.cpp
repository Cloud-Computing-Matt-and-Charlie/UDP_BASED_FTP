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
#include <unistd.h>
#include <streambuf>
#define SEQUENCE_BYTE_NUM 4
#define NUM_SENDING_THREADS 1
#define NUM_RECIEVING_THREADS 1
#define ACK_RESEND_THRESHOLD 3
#define PRINT 1
#define PRINT_R 0
#define NULL_TERMINATOR 0
#define DATA_SEGS 6
#define MAX_CON_SEG 2
int PACKET_SIZE = 3000;
int SEND_PACKET_SIZE = 3000;
pthread_mutex_t DATA_SEG_LOCKS[DATA_SEGS];

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
long vector_bytes_to_int(vector<char> input, long beign, long end);
bool in_between(long i, long a, long b);
void const_int_to_bytes(long input, char* output, int output_size);


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
	SegArgs(vector<UDP*> myUDPs_in, PacketDispenser** myDispenser_in, int data_seg_in,
	        pthread_t* self_in, int offset_in):
		myUDPs{myUDPs_in}, myDispenser{myDispenser_in}, data_seg{data_seg_in},
		self{self_in}, offset{offset_in}
	{};
	vector<UDP*> myUDPs;
	PacketDispenser** myDispenser;
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
	vector<long> bandwidths;
	vector<int> lengths;
	vector<int> disp_loc;
	vector<int> alive;
	int disp_current_index;
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
		//pthread_mutex_init(&this->info_lock, NULL);
		disp_current_index = 0;
		/*
		for (int i = 0; i < DATA_SEGS; i++)
		{
			pthread_mutex_lock(&DATA_SEG_LOCKS[i]);
		}
		*/


	}

	void getAckLocks()
	{
		return;
		pthread_mutex_lock(&this->info_lock);

		for (int i = 0; i < this->myDispensers.size(); i++)
		{
			if (alive[i])
			{


				this->myDispensers[i]->getAckLock();

			}
		}
		pthread_mutex_unlock(&this->info_lock);
		return;
	}

	void releaseAckLocks()
	{
		return;
		pthread_mutex_lock(&this->info_lock);

		for (int i = 0; i < this->myDispensers.size(); i++)
		{
			if (alive[i]) this->myDispensers[i]->releaseAckLock();
		}
		pthread_mutex_unlock(&this->info_lock);
		return;
	}
	/*
	void releaseLocks()
	{
		pthread_mutex_lock(&this->info_lock);
		for (int i = 0; i < alive.size(); i++)
		{
			if (alive[i])
			{
				if (myDispensers[i]->getImDead())
				{
					pthread_mutex_unlock(&DATA_SEG_LOCKS[i]);
					alive[i] = 0;
				}
			}
			else
			{
				pthread_mutex_unlock(&DATA_SEG_LOCKS[i]);
			}
		}
		pthread_mutex_unlock(&this->info_lock);
	}
	void getLocks()
	{
		for (int i = 0; i < DATA_SEGS; i++)
		{
			pthread_mutex_lock(&DATA_SEG_LOCKS[i]);
		}
	}
	*/


	void addPacketDispenser(PacketDispenser* PacketDispenser_in, int length)
	{
		cout<<"Waiting to Add Packet Dispenser"<<endl;
		//pthread_mutex_lock(&this->info_lock);
		cout<<"Packet dispenser added"<<endl;
		this->myDispensers.push_back(PacketDispenser_in);
		this->lengths.push_back(length);
		this->disp_loc.push_back(this->disp_current_index);
		this->alive.push_back(1);
		this->disp_current_index++;
		//pthread_mutex_unlock(&this->info_lock);
		return;
	}
	int getGlobalAllAcksRecieved()
	{

		pthread_mutex_lock(&this->info_lock);
		int temp = 1;
		PacketDispenser* disp;
		for (int i = 0; i < this->myDispensers.size(); i++)
		{
			cout << "MADE IT HERE" << endl;
			if (pthread_mutex_trylock(&DATA_SEG_LOCKS[i]))
			{
				alive[i] = 0;
				pthread_mutex_unlock(&DATA_SEG_LOCKS[i]);
			}
			if (alive[i] == 1)
			{


				disp = this->myDispensers[i];
				temp &= disp->getImDead();
				cout << "Checking to see if " << i << " seg is done" << endl;
				//cout << "Queue size is " << disp->getNumPacketsToSend() << endl;
				if (disp->getImDead() || disp->getAllAcksRecieved())
				{
					cout << "Waiting to kill Dispenser # " << disp_loc[i] << endl;
					this->bandwidths.push_back(disp->getBandwidth());



					pthread_mutex_unlock(&DATA_SEG_LOCKS[i]);

					cout << "Thread " << i << " Killed Noticed In Listener" << endl;
					//delete disp;
					//this->myDispensers.erase(this->myDispensers.begin() + i);
					//this->lengths.erase(this->lengths.begin() + i);
					//this->disp_loc.erase(this->disp_loc.begin() + i);
					this->alive[i] = 0;
					cout << "Thread " << i << " state = " << this->alive[i] << endl;
					int dummy = 0;
					for (auto entry :  this->alive)
					{
						dummy |= entry;
					}
					if (!dummy)
					{
						pthread_mutex_unlock(&this->info_lock);
						return 1;
					}
				}

			}
		}
		pthread_mutex_unlock(&this->info_lock);
		return temp;
	}
	void print_exit()
	{
		long runsum = 0;
		for (auto entry : this->bandwidths) runsum += entry;
		cout << "Bandwidth is " << runsum << endl;
	}
	void globalPutAcks(vector<char> packet_in)
	{
		//this->getAckLocks();
		pthread_mutex_lock(&this->info_lock);
		int count = 0;
		long ack_index;
		int range_loc = 0;
		while (count < packet_in.size())
		{
			ack_index = vector_bytes_to_int(packet_in, count, count + SEQUENCE_BYTE_NUM - 1);
			count += SEQUENCE_BYTE_NUM;
			range_loc = 0;
			if (PRINT_R) cout << "recieved ack # " << ack_index << endl;
			for (int i = 0; i < this->myDispensers.size(); i++)
			{
				if (in_between(ack_index, range_loc, range_loc + this->lengths[i]))
				{
					if (PRINT_R) cout << ack_index << " Belongs inside the " << i << "th Segement" << endl;
					if (alive[i]) this->myDispensers[i]->putAck(ack_index - range_loc);
					break;
				}
				range_loc += this->lengths[i];
			}
		}
		pthread_mutex_unlock(&this->info_lock);
		//this->releaseAckLocks();
		return;
	}
	void* doListen()
	{
		/*
		if (this->myDispensers.size() == 0)
			pthread_mutex_lock(&this->info_lock);
			*/
		vector<char> buffer;
		int working;
		int top;
		int bytes_size;
		char* temp;
		
		while ((!this->getGlobalAllAcksRecieved()) || (this->lengths.size() < DATA_SEGS))
		{
			if (this->lengths.size() == DATA_SEGS)
			{	
				//if (this->getGlobalAllAcksRecieved()) break; 
		
				//cout << "Listen Sleeping Waiting for More Data" << endl;
				//sleep(1);

			}
			if (lengths.size() == 0)
			{
				sleep(1); 
			}
			else
			{
				temp = this->myUDP->recieve(bytes_size);

				buffer = cstring_to_vector(temp, bytes_size);
				//buffer = cstring_to_vector(this->myUDP->recieve(bytes_size), bytes_size);
				if (PRINT_R) cout << "recieved " << vector_bytes_to_int(buffer, 0, 1) << endl;
				this->globalPutAcks(buffer);
			}
		}
		cout << "Exiting Listen with " << this->lengths.size() << " Segments Complete" << endl;
		this->print_exit();

		pthread_exit(NULL);


	}
	static void* threadLauncher(void* input)
	{
		cout << "Launching Listen Thread" << endl;
		return ((ListenThread*)(input))->doListen();
	}

};

void* launch_segement_threads(void* input_param)
{

	SegArgs* mySegArgs = (SegArgs*)(input_param);
	PacketDispenser* sessionPacketDispenser = *(mySegArgs->myDispenser);
	vector<UDP*> sessionUDPs = mySegArgs->myUDPs;
	int data_seg = mySegArgs->data_seg;
	long offset = mySegArgs->offset;
	pthread_mutex_lock(&DATA_SEG_LOCKS[data_seg]);

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
	for (int i = 0; i < sending_threads.size(); i++)
	{
		delete sending_threads[i];
	}
	cout << "Exiting Segement Launcher: Waiting For Lock" << endl;


	delete sessionPacketDispenser;

	pthread_mutex_unlock(&DATA_SEG_LOCKS[data_seg]);
	cout << "Exiting Segement Launcher: Complete" << endl;
	pthread_exit(NULL);

}


void* sender_thread_function(void* input_param)
{

	ThreadArgs* myThreadArgs = (ThreadArgs*)(input_param);

	vector<char> temp;
	char* c_string_buffer;
	long num_temp;
	char num_buffer[SEQUENCE_BYTE_NUM];
	int bytes_back;
	//while (!myThreadArgs->myDispenser->getAllAcksRecieved())
	while (!myThreadArgs->myDispenser->getImDead())
	{

		temp = myThreadArgs->myDispenser->getPacket();
		if (temp.empty()) break;

		if (!temp.empty())
		{
			//add_offset(temp, myThreadArgs->offset);

			num_temp = vector_bytes_to_int(temp, 0, SEQUENCE_BYTE_NUM - 1);
			num_temp = num_temp + (myThreadArgs->offset);

			const_int_to_bytes(num_temp, num_buffer, SEQUENCE_BYTE_NUM);
			for (int i = 0; i < SEQUENCE_BYTE_NUM; i++)
			{
				temp[i] = num_buffer[i];
			}

			if (temp.size() < SEND_PACKET_SIZE)
			{
				myThreadArgs->myUDP->setSendPacketSize(temp.size());
				myThreadArgs->myUDP->send(vector_to_cstring(temp));
				myThreadArgs->myUDP->setSendPacketSize(SEND_PACKET_SIZE);
			}
			else
				myThreadArgs->myUDP->send(vector_to_cstring(temp));
			//long num_temp = vector_bytes_to_int(temp, 0, SEQUENCE_BYTE_NUM - 1);


			if (PRINT) cout << "Segement # " << myThreadArgs->data_seg << endl;
			if (PRINT) cout << "Thread #: " << myThreadArgs->id;
			if (PRINT) cout << " Packet #: " << num_temp << endl;
			if (PRINT) cout << "Effective Packet #: " << ((long)myThreadArgs->offset + num_temp) << endl;
			if (PRINT) cout << "Current Bandwidth " << myThreadArgs->myDispenser->getBandwidth() << endl;
			if (PRINT) cout << "Time since last packet " <<
				                myThreadArgs->myDispenser->getTimeSinceLastPacket() << endl;
			if ((myThreadArgs->myDispenser->getNumPacketsSent() > 12130) && 0)
			{
				cout << "Segement # " << myThreadArgs->data_seg << endl;
				cout << "Thread #: " << myThreadArgs->id;
				cout << " Packet #: " << num_temp << endl;
				cout << "Effective Packet #: " << (myThreadArgs->offset + num_temp) << endl;
				cout << "Current Bandwidth " << myThreadArgs->myDispenser->getBandwidth() << endl;
				cout << "Time since last packet " <<
				     myThreadArgs->myDispenser->getTimeSinceLastPacket() << endl;
			}

		}
		if ((myThreadArgs->myDispenser->getNumPacketsSent() >= (2 * myThreadArgs->myDispenser->getTotalPackets())))
		{

			break;
		}
		if ((myThreadArgs->myDispenser->getAllAcksRecieved())) break;
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

	int count = 0;
	vector<char> working(packet_size);
	//unsigned char* bytes;
	int bytes_returned;
	int null_terminator = 0;
	int data_packet_size = packet_size - (sequencing_bytes + NULL_TERMINATOR);
	int remainder;
	char bytes[SEQUENCE_BYTE_NUM];
	char* cstring_buff = new char[packet_size];
	while (((count)*data_packet_size) <= total_bytes)
	{
		remainder = total_bytes - (count * data_packet_size);
		const_int_to_bytes(count, bytes, SEQUENCE_BYTE_NUM);
		for (int i = 0; i < sequencing_bytes; i++)
			cstring_buff[i] = bytes[i];
		if (remainder >= data_packet_size)
		{
			input_file.read((cstring_buff + sequencing_bytes), data_packet_size);
			if (NULL_TERMINATOR)
			{
				cstring_buff[packet_size - 1] = '\0';
			}
			output.push_back(cstring_to_vector(cstring_buff, packet_size));
		}
		else
		{
			input_file.read((cstring_buff + sequencing_bytes), remainder);
			if (NULL_TERMINATOR)
			{
				cstring_buff[remainder - 1] = '\0';
			}
			output.push_back(cstring_to_vector(cstring_buff, remainder));
		}


		count++;
	}
	delete [] cstring_buff;
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
	for (int i = 0; i < DATA_SEGS; i++)
	{
		pthread_mutex_init(&DATA_SEG_LOCKS[i], NULL);

	}

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
		sessionUDPs[i]->setPacketSize(PACKET_SIZE);
		sessionUDPs[i]->setSendPacketSize(SEND_PACKET_SIZE);
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
		if (i != (DATA_SEGS - 1))
			seg_lengths.push_back(seg_length);
		else
		{

			seg_lengths.push_back(file_length - (seg_length * (DATA_SEGS - 1)));
			cout << "HELLO" << endl;
		}
	}
	cout << "Using " << DATA_SEGS << " data segements";
	int dum = 0;
	for (auto entry : seg_lengths)
	{
		cout << "Segement " << dum << " contains " << entry << " bytes " << endl;
		dum++;
	}
	vector<vector<vector<char>>> raw_datas(DATA_SEGS);
	pthread_t* temp_p_thread;
	vector<SegArgs*> sessionSegArgs;
	SegArgs* tempSegArgs;
	int rc;
	int offset = 0;
	int joined = 0;
	vector<pthread_t*> seg_threads;
	pthread_t listen_pid;


	pthread_attr_t data_seg_attributes[DATA_SEGS];

	ListenThread* sessionListenThread = new ListenThread(sessionUDPs[0], &listen_pid);
	//sessionListenThread->getLocks();
	rc = pthread_create(&listen_pid, NULL, sessionListenThread->threadLauncher, ((void*)sessionListenThread));
	int threads_active_count = 0;
	for (int i = 0; i < DATA_SEGS; i++)
	{
		pthread_attr_init(&data_seg_attributes[i]);
		pthread_attr_setdetachstate(&data_seg_attributes[i], PTHREAD_CREATE_JOINABLE);
		cout << "Reading From File: Main" << endl;
		temp_p_thread = new pthread_t;
		read_from_file(fl, SEND_PACKET_SIZE, SEQUENCE_BYTE_NUM, raw_datas[i], seg_lengths[i]);

		cout << "legnth of data " << i << " is " << raw_datas[i].size() << endl;
		PacketDispenser* sessionPacketDispenser = new PacketDispenser(raw_datas[i]);
		//sessionPacketDispenser->setMaxBandwidth(1000000);
		if (threads_active_count >= MAX_CON_SEG)
		{
			cout << "Joining thread # " << joined << endl;

			//pthread_mutex_lock(&DATA_SEG_LOCKS[i]);
			//pthread_mutex_unlock(&DATA_SEG_LOCKS[i]);
			/*
			while (!pthread_mutex_trylock(&DATA_SEG_LOCKS[i]))
			{

				pthread_mutex_unlock(&DATA_SEG_LOCKS[i]);
			}
			*/
			pthread_join(*seg_threads[joined], NULL);
			cout << "Begin: Joining thread # " << joined << endl;

			cout << "Unlocked Mutex " << joined << endl;
			joined++;
			threads_active_count--;
			cout << "End: Joined thread # " << joined << endl;
		}
		tempSegArgs = new SegArgs(sessionUDPs, &sessionPacketDispenser, i,
		                          temp_p_thread, offset);
		sessionListenThread->addPacketDispenser(sessionPacketDispenser, raw_datas[i].size());
		sessionSegArgs.push_back(tempSegArgs);
		rc = pthread_create(tempSegArgs->self, &data_seg_attributes[i], launch_segement_threads,
		                    ((void*)tempSegArgs));
		seg_threads.push_back(tempSegArgs->self);
		offset += raw_datas[i].size();
		raw_datas[i].clear();
		raw_datas[i].shrink_to_fit();
		threads_active_count++;

	}
	cout << "All Segements Created " << endl;
	fl.close();
	cout << "Done Creating, Joining Threads " << endl;
	//sleep(1);
	//sessionListenThread->releaseLocks();
	for (int i = joined; i < DATA_SEGS; i++)
	{
		/*
		while (!pthread_mutex_trylock(&DATA_SEG_LOCKS[i]))
		{
			sessionListenThread->releaseLocks();

		}
		*/
		pthread_join(*seg_threads[i], NULL);
	}
	for (int i = 0; i < sessionSegArgs.size(); i++)
	{
		delete sessionSegArgs[i];
	}
	for (int i = 0; i < sessionUDPs.size(); i++)
	{
		delete sessionUDPs[i];
	}

	for (int i = 0; i < seg_threads.size(); i++)
	{
		delete seg_threads[i];
	}


}


bool in_between(long i,  long a, long b)
{
	if (i >= a)
	{
		if (i < b) return true;
	}
	return false;
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

void const_int_to_bytes(long input, char* output, int output_size)
{

	int bytes = output_size;
	for ( int i = 0; i < bytes; i++)
	{
		// (*output)[i] = (0xFF & input >> (8 * (bytes - i - 1)));
		output[i] = (0xFF & (input >> (8 * (bytes - i - 1))));


	}
	output_size = bytes;
	return;

}



