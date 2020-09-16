#ifndef CLIENT
#define CLIENT

class client_listen : public UDP
{
public:
    //attrributes
    char *data_array;
    int num_packets_expected, packet_size, array_size;
    bool first_packet;
    vector<int> packet_ID_list;
    int packet_ID_list_size;

    //functions
    client_listen(char* dest_ip_address, int listen_port, int dest_port);
    void create_array(int packet_size, int num_packets);
    void array_add(char * data, int packet_offset, int data_size);
    void print_data_array();
    int strip_header(char * data);
    void process_packet(char * data, int size);
    ~client();
};

#endif