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
  pthread_mutex_init(&this->pop_lock);
  pthread_mutex_init(&this->push_lock);
  pthread_mutex_init(&this->ack_lock);
  pthread_muxtex_unlock(&this->pop_lock);
  pthread_mutex_unlock(&this->push_lock);
  pthread_mutex_unlock(&this->ack_lock);
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
void PacketDispenser::setTimeSinceLastPacket()
{

  while (this->getTimeSinceLastPacket() < this->min_diff_time) {}
  this->last_packet_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

string PacketDispenser::getPacket()
{
  pthread_mutex_lock(&pop_lock);
  string output_data = this->packet_queue.pop();
  while (is_acked[output_data.sequence_number])
  {
    output_data = this->packet_queue.pop();
  }
  //auto time_now = std::chrono::system_clock::now();
  //this->last_packet_time = std::chrono::system_clock::to_time_t(time_now);
  this->packets_sent++;
  this->setTimeSinceLastPacket();
  pthread_mutex_unlock(&pop_lock);
  return output_data.data;
}

int PacketDistpenser::getBandwidth()
{
  this->current_bandwidth = int(double(this->packets_sent) / this->getTotalTime());
  return this->current_bandwidth;
}

void PacketDispenser::setMaxBandwidth(int max_bandwidth_in)
{
  this->max_bandwidth = max_bandwidth_in;
  this->min_diff_time = 1 / ((double)max_bandwidth_in);
}
void PacketDispenser::getAckDistance()
{
  for (int i = this->is_acked.size() - 1; i >= 0; i--)
  {
    if (is_acked[i])
    {
      int last_acked = i + 1;
      break;
    }
  }
  return packets_sent - last_acked;
}
void PacketDispenser::putAck(int sequence_number)
{
  pthread_mutex_lock(&ack_lock);
  this->is_acked(sequence_number) = 1;
  pthread mutex_unlock(&ack_lock);
}

int PacketDispenser::getNumPacketsToSend()
{
  return this->packet_queue.size();
}
void PacketDispenser::resendInRange(int begin, int end)
{
  pthread_mutex_lock(&push_lock);
  queue_node* temp;
  for (int i = begin; i < end + 1; i++)
  {
    if (!is_acked[i])
    {
      temp = new queue_node(input_data[i], i);
      packet_queue.push(temp);
    }
  }
  pthread_mutex_unlock(&push_lock);
}
void PacketDispenser::addDataToSend(vector<int> new_data)
{
  pthread_mutex_lock(&this->push_lock);
  int count = this->input_data.size();
  queue_node* temp;
  for (auto entry : new_data)
  {
    this->input_data.push_back(entry);
    this->is_acked.push_back(0);
    temp = new queue_node(entry, count++);
    this->packet_queue.push(temp);
  }
  pthread_mutex_unlock(&this->push_lock);
}

PacketDispenser::~PacketDispenser = default;






