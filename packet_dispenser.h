#include<iostream>
#include<pthread.h>
using namespace std;

#define WINDOW_SIZE 20



class PacketDispenser
{
private: 
  pthread_mutex_t pop_lock; 
  pthread_mutex_t push_lock; 
  vector<string> input_data; 
  vector<int> is_acked; 
  int max_bandwidth; 
  
  struct queue_node(string payload, int index) 
  {
    string data = payload; 
    int sequence_number = index;
  };
  queue<*queue_node> packet_queue; 
  int window_size = 20; //how long until we start re-queueing 
public: 
  PacketDispenser(vector<string> raw_input_data); 
  string getPacket(); 
  void putAck(int sequence_number);
  int getBandwidth(); 
  void setBandwidth(int bandwidth); 
  ~PacketDispenser(); 
};
