#ifndef packet_dispenser_H
#define packet_dispenser_H

#include<vector>
#include <queue>
#include<iostream>
#include<pthread.h>
#include<string>
#include<chrono>

using namespace std;

#define WINDOW_SIZE 20



class PacketDispenser
{
private:
  pthread_mutex_t queue_lock;
  pthread_mutex_t ack_lock;
  pthread_mutex_t dead_lock;
  vector<vector<char>> input_data;
  vector<int> is_acked;
  int long max_bandwidth;
  int current_bandwidth;
  long packets_sent;
  std::chrono::time_point<std::chrono::system_clock> total_start;
  std::chrono::time_point<std::chrono::system_clock> last_packet_time;
  std::chrono::time_point<std::chrono::system_clock> current_time;

  long max_num_packets_sent;

  double min_diff_time;
  int packet_size;
  int all_acks_recieved;
  int is_dead;

  struct queue_node
  {
    queue_node(vector<char> payload, int index) : data{payload}, sequence_number{index} {}
    vector<char> data;
    int sequence_number;
    queue_node& operator=(const queue_node& other);
  };

  queue<queue_node*> packet_queue;
  int window_size = 20; //how long until we start re-queueing
public:


  PacketDispenser(vector<vector<char>> raw_input_data);
  vector<char> getPacket();
  void putAck(long sequence_number);
  long getBandwidth();
  void setMaxBandwidth(int bandwidth);
  double getTimeSinceLastPacket();
  void setTimeSinceLastPacket();
  int getAckDistance();
  void resendInRange(int start, int stop);
  int getNumPacketsToSend();
  void addDataToSend(vector<vector<char>> new_data);
  double getTotalTime();
  int getNumPacketsSent();
  void resendAll();
  int getAllAcksRecieved();
  void getAckLock();
  void releaseAckLock();
  void getQueueLock();
  int getTotalPackets();
  void releaseQueueLock();
  void resendOnTheshold(int threshold);
  void setImDead();
  int getImDead();


  //~PacketDispenser();

};

#endif
