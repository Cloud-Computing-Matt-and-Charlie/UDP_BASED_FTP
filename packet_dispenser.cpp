
#include<queue>
#include<vector>
#include<iostream>
#include<pthread.h>
#include <string.h>
#include <cmath>
#include<chrono>
#include "packet_dispenser.h"
#define PRINT_ACKS 1
using namespace std;

PacketDispenser::PacketDispenser(vector<vector<char>> raw_input_data) : input_data{raw_input_data}, packets_sent(0), min_diff_time(0)
{
  this->is_acked = vector<int>(input_data.size(), 0);
  this->all_acks_recieved = 0;
  queue_node* temp;
  int count = 0;
  //enqueue the list
  this->packet_size = input_data[0].size();
  for (auto entry : this->input_data)
  {
    temp = new queue_node(entry, count++);
    this->packet_queue.push(temp);

  }

  this->total_start = std::chrono::system_clock::now();
  this->last_packet_time = this->total_start;
  pthread_mutex_init(&this->queue_lock, NULL);
  pthread_mutex_init(&this->ack_lock, NULL);
  pthread_mutex_unlock(&this->queue_lock);
  pthread_mutex_unlock(&this->ack_lock);


}

double PacketDispenser::getTimeSinceLastPacket()
{

  auto time_now = std::chrono::system_clock::now();
  return (((double)std::chrono::duration_cast<std::chrono::milliseconds>(time_now - this->last_packet_time).count()) / 1000);
}

double PacketDispenser::getTotalTime()
{
  auto time_now = std::chrono::system_clock::now();
  return (((double)std::chrono::duration_cast<std::chrono::milliseconds>(time_now - this->total_start).count()) / 1000);
}
void PacketDispenser::setTimeSinceLastPacket()
{
  while (this->getTimeSinceLastPacket() < this->min_diff_time) {}
  this->last_packet_time = std::chrono::system_clock::now();
}
vector<char> PacketDispenser::getPacket()
{

  queue_node* output_data;
  if (this->packet_queue.size())
  {
    pthread_mutex_lock(&queue_lock);
    output_data = this->packet_queue.front();
    this->packet_queue.pop();
    pthread_mutex_unlock(&queue_lock);
    while (is_acked[output_data->sequence_number])
    {
      free(output_data);
      if (this->packet_queue.size() == 0)
      {
        this->resendAll();
        if (this->packet_queue.size() == 0)
        {
          return {};
        }
      }
      pthread_mutex_lock(&queue_lock);
      output_data = this->packet_queue.front();
      this->packet_queue.pop();
      pthread_mutex_unlock(&queue_lock);

    }
    this->packets_sent++;
    this->setTimeSinceLastPacket();


  }
  else
  {

    this->resendAll();
    if (this->packet_queue.size() == 0)
    {
      return {};
    }
    else return this->getPacket();
  }

  //auto time_now = std::chrono::system_clock::now();
  //this->last_packet_time = std::chrono::system_clock::to_time_t(time_now);
  return output_data->data;
}
/*
vector<char> PacketDispenser::getPacket()
{
  pthread_mutex_lock(&queue_lock);
  queue_node* output_data;
  if (this->packet_queue.size())
  {
    output_data = this->packet_queue.front();
    this->packet_queue.pop();
    while (is_acked[output_data->sequence_number])
    {
      free(output_data);
      if (this->packet_queue.size() == 0)
      {
        this->resendAll();
        if (this->packet_queue.size() == 0)
        {
          pthread_mutex_unlock(&queue_lock);
          return {};
        }
      }
      output_data = this->packet_queue.front();
      this->packet_queue.pop();

    }
    this->packets_sent++;
    this->setTimeSinceLastPacket();


  }
  else
  {

    this->resendAll();
    pthread_mutex_unlock(&queue_lock);
    if (this->packet_queue.size() == 0)
    {
      return {};
    }
    else return this->getPacket();
  }

  //auto time_now = std::chrono::system_clock::now();
  //this->last_packet_time = std::chrono::system_clock::to_time_t(time_now);

  pthread_mutex_unlock(&queue_lock);
  return output_data->data;
}
*/

unsigned long PacketDispenser::getBandwidth()
{
  this->current_bandwidth = int(((double)(this->packets_sent) * this->packet_size) / this->getTotalTime());
  return this->current_bandwidth;
}

void PacketDispenser::setMaxBandwidth(int max_bandwidth_in)
{
  this->max_bandwidth = max_bandwidth_in;
  double max_packet_bandwidth = (this->max_bandwidth / this->packet_size);
  this->min_diff_time = 1 / ((double)max_bandwidth_in);
  cout << "MIN DIFF TIME IS NOW " << min_diff_time << endl;
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
void PacketDispenser::getAckLock()
{
  pthread_mutex_lock(&ack_lock);
}
void PacketDispenser::releaseAckLock()
{
  pthread_mutex_unlock(&ack_lock);
}
void PacketDispenser::putAck(unsigned long sequence_number)
{
  if ((sequence_number > input_data.size()) || (sequence_number > this->packets_sent)
      || (sequence_number < 0))
  {
    cout << "Error Attempted Ack For Invalid Sequence Number" << endl;
  }
  else
  {
    this->is_acked[sequence_number] = 1;
    if (this->all_acks_recieved)
    {
      cout << endl << endl << endl << "ALL ACKS RECIEVED" << endl << endl << endl;
    }
  }
  int ack_temp = 1;
  int debug_sum = 0;
  for (auto entry : this->is_acked)
  {
    if (entry == 0) ack_temp = 0;
    debug_sum += (entry);
  }
  this->all_acks_recieved = ack_temp;
  if (PRINT_ACKS)
  {
    cout << " HAVE " << debug_sum << "ACKS of " << this->is_acked.size() << endl;
    if (this->all_acks_recieved)
    {
      cout << endl << endl << endl << "ALL ACKS RECIEVED" << endl << endl << endl;
    }
  }
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
  pthread_mutex_lock(&queue_lock);
  pthread_mutex_lock(&ack_lock);
  queue_node* temp;
  for (int i = begin; i < end + 1; i++)
  {
    if (!is_acked[i])
    {
      temp = new queue_node(input_data[i], i);
      packet_queue.push(temp);
    }
  }
  pthread_mutex_unlock(&queue_lock);
  pthread_mutex_unlock(&ack_lock);
}

void PacketDispenser::resendAll()
{
  pthread_mutex_lock(&queue_lock);
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
  pthread_mutex_unlock(&queue_lock);
}

void PacketDispenser::resendOnTheshold(int threshold)
{
  if ((this->packet_queue.size() < (this->input_data.size() / threshold))
      && (!this->all_acks_recieved) &&
      (this->packet_queue.size() < this->input_data.size()))
  {
    this->resendAll();
  }
}
void PacketDispenser::addDataToSend(vector<vector<char>> new_data)
{
  pthread_mutex_lock(&this->queue_lock);
  int count = this->input_data.size();
  queue_node* temp;
  for (auto entry : new_data)
  {
    this->input_data.push_back(entry);
    this->is_acked.push_back(0);
    temp = new queue_node(entry, count++);
    this->packet_queue.push(temp);
  }
  pthread_mutex_unlock(&this->queue_lock);
}

int PacketDispenser::getAllAcksRecieved()
{
  //pthread_mutex_lock(&ack_lock);
  return this->all_acks_recieved;
  //pthread_mutex_unlock(&ack_lock);
}

//PacketDispenser::~PacketDispenser = default;

void PacketDispenser::getQueueLock()
{
  pthread_mutex_lock(&this->queue_lock);
}
void PacketDispenser::releaseQueueLock()
{
  pthread_mutex_unlock(&this->queue_lock);
}




