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
    // std::map<int, std::vector<char>> data_map;
    std::map<int, char> data_map;
    int num_packets_expected, num_packets_received;//, packet_size;
    bool first_packet;
    std::queue<std::vector<char>> packet_ID_list;
    std::queue< std::vector<char> > packet_queue;
    std::queue< std::vector<char> > packets_for_write;
    std::vector< std::vector<char> > ACK_queue;
    int packet_ID_list_size;
    pthread_mutex_t packet_lock;
    char * file_name;

    //functions
    client_listen(char* dest_ip_address, char * listen_port, char * dest_port, char * output_file);
    // void map_add(int packet_number, std::vector<char> data);
    // void print_data_map();
    int strip_header(std::vector<char> &data);
    void control_packet(std::vector<char> data);
    void create_ACK_packet(int ACK_packet_size);
    void process_packet(std::vector<char> packet);
    void send_ACKs(int index);
    ~client_listen();
};

void * empty_send_queue(void * input);
void* empty_data_queue(void* input);
std::vector<char> cstring_to_vector(char* input, int size);
char* vector_to_cstring(std::vector<char> input);
void listener(char* dest_ip_address, char* listen_port, char* dest_port, char * output_file);
long vector_bytes_to_int(std::vector<char> input, long start, long end);
void file_reader();
#endif