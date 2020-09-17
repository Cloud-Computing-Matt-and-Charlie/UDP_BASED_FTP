#ifndef packet_dispenser_H
#define packet_dispenser_H

#include<vector>
#include <queue>
#include<iostream>
#include<pthread.h>
#include<string.h>
using namespace std;

#define WINDOW_SIZE 20



class PacketDispenser
{
private:
  pthread_mutex_t pop_lock;
  pthread_mutex_t push_lock;
  pthread_mutex_t ack_lock;
  vector<string> input_data;
  vector<int> is_acked;
  int long max_bandwidth;
  int current_bandwidth;
  long int packets_sent;
  time_t total_start;
  time_t last_packet_time;
  double min_diff_time;
  struct queue_node
  {
    queue_node(string payload, int index) : data{payload}, sequence_number{index} {}
    string data;
    int sequence_number;
  };

  queue<queue_node* > packet_queue;
  int window_size = 20; //how long until we start re-queueing
public:

  PacketDispenser(vector<string> raw_input_data);
  string getPacket();
  void putAck(int sequence_number);
  int getBandwidth();
  void setMaxBandwidth(int bandwidth);
  double getTimeSinceLastPacket();
  void setTimeSinceLastPacket();
  int getAckDistance();
  void resendInRange(int start, int stop);
  int getNumPacketsToSend();
  void addDataToSend(vector<string> new_data);
  double getTotalTime();
  ~PacketDispenser();

};

#endif
