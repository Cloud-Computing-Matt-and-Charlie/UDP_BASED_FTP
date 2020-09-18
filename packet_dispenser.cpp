
#include<queue>
#include<vector>
#include<iostream>
#include<pthread.h>
#include <string.h>
#include <cmath>
#include "packet_dispenser.h"
using namespace std;

PacketDispenser::PacketDispenser(vector<vector<char>> raw_input_data) : input_data{raw_input_data}, packets_sent(0), min_diff_time(0)
{
  this->is_acked = vector<int>(input_data.size(), 0);
  queue_node* temp;
  int count = 0;
  //enqueue the list
  this->packet_size = input_data[0].size();
  for (auto entry : this->input_data)
  {
    temp = new queue_node(entry, count++);
    this->packet_queue.push(temp);

  }
  auto start = std::chrono::system_clock::now();
  this->total_start = std::chrono::system_clock::to_time_t(start);
  this->last_packet_time = this->total_start;
  pthread_mutex_init(&this->pop_lock, NULL);
  pthread_mutex_init(&this->push_lock, NULL);
  pthread_mutex_init(&this->ack_lock, NULL);
  pthread_mutex_unlock(&this->pop_lock);
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

vector<char> PacketDispenser::getPacket()
{
  pthread_mutex_lock(&pop_lock);
  queue_node* output_data = this->packet_queue.front();
  this->packet_queue.pop();
  while (is_acked[output_data->sequence_number])
  {
    //free(output_data); class has default destructor
    output_data = this->packet_queue.front();
    this->packet_queue.pop();
  }
  //auto time_now = std::chrono::system_clock::now();
  //this->last_packet_time = std::chrono::system_clock::to_time_t(time_now);
  this->packets_sent++;
  this->setTimeSinceLastPacket();
  pthread_mutex_unlock(&pop_lock);
  return output_data->data;
}

int PacketDispenser::getBandwidth()
{
  this->current_bandwidth = int(double(this->packets_sent) / this->getTotalTime());
  return this->current_bandwidth;
}

void PacketDispenser::setMaxBandwidth(int max_bandwidth_in)
{
  this->max_bandwidth = max_bandwidth_in;
  double max_packet_bandwidth = (this->max_bandwidth / this->packet_size);
  this->min_diff_time = 1 / ((double)max_bandwidth_in);
}
int PacketDispenser::getAckDistance()
{
  int last_acked = 0;
  for (int i = this->is_acked.size() - 1; i >= 0; i--)
  {
    if (is_acked[i])
    {
      last_acked = i + 1;
      break;
    }
  }
  return packets_sent - last_acked;
}
void PacketDispenser::putAck(int sequence_number)
{
  pthread_mutex_lock(&ack_lock);
  if (sequence_number > input_data.size())
  {
    cout << "Error Attempted Ack For Invalid Sequence Number" << endl;
  }
  else this->is_acked[sequence_number] = 1;
  pthread_mutex_unlock(&ack_lock);
}




int PacketDispenser::getNumPacketsSent()
{
  return this->packets_sent;
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

void PacketDispenser::resendAll()
{
  pthread_mutex_lock(&push_lock);
  queue_node* temp;
  int range_max = min((int)this->packets_sent, (int)this->input_data.size());
  for (int i = 0; i < range_max; i++)
  {
    if (!is_acked[i])
    {
      temp = new queue_node(input_data[i], i);
      packet_queue.push(temp);
    }
  }
  pthread_mutex_unlock(&push_lock);
}
void PacketDispenser::addDataToSend(vector<vector<char>> new_data)
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

//PacketDispenser::~PacketDispenser = default;






