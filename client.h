#ifndef CLIENT
#define CLIENT

#include "UDP.h"
#include <vector>
#include <queue>

class client_listen : public UDP
{
public:
    //attrributes
    char *data_array;
    int num_packets_expected, packet_size, array_size;
    bool first_packet;
    std::vector<int> packet_ID_list;
    std::queue<std::string> packet_queue;
    int packet_ID_list_size;
    pthread_mutex_t packet_lock;

    //functions
    // client_listen(char* dest_ip_address, int listen_port, int dest_port):UDP(dest_ip_address,listen_port,dest_port){};
    client_listen(char* dest_ip_address, char * listen_port, int dest_port);
    void create_array(int packet_size, int num_packets);
    void array_add(std::string data, int packet_offset, int data_size);
    void print_data_array();
    int strip_header(std::string data);
    void control_packet(std::string data);
    // void process_packet(char * data, int size);
    static void * do_processing(void * arg);
    // void * process_packet(void * packet_struct);
    void process_packet(std::string packet, int size);
    void empty_packet_queue();
    // ~client_listen();
};

#endif