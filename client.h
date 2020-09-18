#ifndef CLIENT
#define CLIENT

#include "UDP.h"
#include <vector>
#include <queue>
#include <map>

class client_listen : public UDP
{
public:
    //attrributes
    // char *data_array;
    std::map<int, std::vector<char>> data_map;
    int num_packets_expected, packet_size;
    bool first_packet;
    // std::vector<char *> packet_ID_list;
    std::queue<std::vector<char>> packet_ID_list;
    std::queue< std::vector<char> > packet_queue;
    // std::queue< std::vector<char> > ACK_queue;
    std::vector< std::vector<char> > ACK_queue;
    int packet_ID_list_size;
    pthread_mutex_t packet_lock;

    //functions
    // client_listen(char* dest_ip_address, int listen_port, int dest_port):UDP(dest_ip_address,listen_port,dest_port){};
    client_listen(char* dest_ip_address, char * listen_port, char * dest_port);
    // void create_array(int packet_size, int num_packets);
    void map_add(int packet_number, std::vector<char> data);
    void print_data_map();
    int strip_header(std::vector<char> data);
    void control_packet(std::string data);
    void create_ACK_packet();
    // void process_packet(char * data, int size);
    static void * do_processing(void * arg);
    // void * process_packet(void * packet_struct);
    void process_packet(std::vector<char> packet);
    void send_ACKs(int index);
    // ~client_listen();
};

void * empty_packet_queue(void * input);
std::vector<char> cstring_to_vector(char* input, int size);
char* vector_to_cstring(std::vector<char> input);

#endif