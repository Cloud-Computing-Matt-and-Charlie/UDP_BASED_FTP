#include<iostream>
#include<pthread.h>
using namespace std; 

PacketDispenser::PacketDispenser(vector<string> raw_input_data) : input_data{raw_input_data}, packets_sent(0), min_diff_time(1000)
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
    auto start = std::chrono::system_clock::now();
    this->total_start = std::chrono::system_clock::to_time_t(start);
    this->last_packet_time = this->total_start; 
    pthread_mutex_init(&pop_lock); 
    pthread_mutex_init(&push_lock); 
    pthread_muxtex_unlock(&pop_lock); 
    pthread_mutex_unlock(&push_lock); 
}

double PacketDispenser::getTimeSinceLastPacket()
{
    time_t time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
    return difftime(time_now, this->last_packet_time); 
}

double PacketDispenser::getTotalTime()
{
  time_t time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
  return difftime(time_now, this->total_start); 
}
void PacketDispenser:setTimeSinceLastPacket()
{
  
  while(this->getTimeSinceLastPacket() < this->min_diff_time) {}
  this->last_packet_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
}
  
string PacketDispenser::getPacket()
{
  pthread_mutex_lock(&pop_lock); 
  string output_data = this->packet_queue.pop(); 
  auto time_now = std::chrono::system_clock::now(); 
  this->last_packet_time = std::chrono::system_clock::to_time_t(time_now);
  this->packets_sent++; 
  this->setTimeSinceLastPacket(); 
  pthread_mutex_unlock(&pop_lock); 
}

int PacketDistpenser::getBandwidth()
{
  this->current_bandwidth = int(double(this->packets_sent)/this->getTotalTime()); 
  return this->current_bandwidth; 
}
 
void PacketDispenser::setMaxBandwidth(int max_bandwidth_in)
{
  this->max_bandwidth = max_bandwidth_in; 
  this->min_diff_time = 1/((double)max_bandwidth_in); 
}

string Packet 
  
  
  
  

/*

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

  */  
    
    
