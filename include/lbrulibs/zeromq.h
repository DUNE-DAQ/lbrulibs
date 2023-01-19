#ifndef LBRULIBS_INCLUDE_LBRULIBS_ZEROMQ_H
#define LBRULIBS_INCLUDE_LBRULIBS_ZEROMQ_H
#include <vector>
#include <string>

class zeromq {
  public:
    zeromq();
    zeromq(std::vector<std::string> ipaddresses, bool req_stream);
    zeromq(std::string ipaddress, bool req_stream, void* data_input, int data_size);
    zeromq(int port, bool rep_stream, void* buf, int data_size);
    ~zeromq();
    bool stream;
    std::vector<void*> _Context_vect;
    std::vector<void*> _Socket_vect;
    std::vector<std::string> _IPaddress_vect;
    std::vector<void*> _Socketserver_vect;
    uint32_t _UserID;
    std::string int_to_str(int myValue);
    void makeclients(std::vector<int> ipadd_locs);
    void makeservers(std::vector<int> ports);
    void send(bool client, void* input, int datasize, unsigned short whichConnection);
    int recv(bool client, void*idbuf, void* buffer, int datasize, unsigned short whichConnection);
    void setconnectionID(uint32_t inputid);
    void connect_id(int enabledLoc);
    void connect(int enabledLoc, bool stream);
    void clientconnect(int enabledLoc, bool stream);
    void send_recv(int socketnum, void *input, void* output, int data_size);
    void close(int index);
    void connect_send_receive_close(void* input, void* output, int data_size, int num_data, std::string ipaddress);
    void server(void* buffer, void* idbuf, std::vector<int> ports, int data_size, bool stream);
    void send_id(int socketnum, void* input, int data_size);
    void send_new(int socketnum, void* input, int data_size);
    void send_multi(int socketnum, void* input, int data_size);
};

#endif
