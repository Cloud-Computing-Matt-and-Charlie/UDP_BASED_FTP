#include<iostream>
#include<pthread.h>
using namespace std; 

class PacketDispenser
{
private: 
  pthread_mutex_t pop_lock; 
  pthread_mutex_t push_lock; 
  vector<string> input_data; 
  vector<int> is_acked; 
  
  struct queue_node(string payload, int index) 
  {
    string data = payload; 
    int sequence_number = index; 
  };
  queue<*queue_node> packet_queue; 
  
  int window_size = 20; //how long until we start re-queueing 
public: 
  PacketDispenser(vector<string> raw_input_data) : input_data{raw_input_data}
  {
    this->is_acked = vector<int>(input_data.size(), 0); 
    queue_node* temp; 
    int count = 0; 
    //enqueue the list 
    for (auto entry : this->input_data)
    {
      temp = new queue_node(entry, count++);  
      this->packet_queue.push(temp); 
    }

    
    
    
