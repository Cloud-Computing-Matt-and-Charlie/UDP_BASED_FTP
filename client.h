#ifndef CLIENT
#define CLIENT

#include "UDP.h"
#include <vector>

class client_listen : public UDP
{
public:
    //attrributes
    char *data_array;
    int num_packets_expected, packet_size, array_size;
    bool first_packet;
    std::vector<int> packet_ID_list;
    int packet_ID_list_size;
    pthread_mutex_t mem_lock;

    //functions
    // client_listen(char* dest_ip_address, int listen_port, int dest_port):UDP(dest_ip_address,listen_port,dest_port){};
    client_listen(char* dest_ip_address, char * listen_port, int dest_port);
    void create_array(int packet_size, int num_packets);
    void array_add(char * data, int packet_offset, int data_size);
    void print_data_array();
    int strip_header(char * data);
    void control_packet(char * data);
    // void process_packet(char * data, int size);
    static void * do_processing(void * arg);
    void * process_packet(void * packet_struct);
    // ~client_listen();
};

#endif