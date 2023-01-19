#include <zmq.h>
#include <unistd.h>
#include <iostream>
#include "lbrulibs/zeromq.h"

zeromq::zeromq(){
 stream = 1;
}

zeromq::zeromq(std::vector<std::string> ipaddresses, bool req_stream){
  _IPaddress_vect = ipaddresses;
  stream = req_stream;
}

//client constructor
zeromq::zeromq(std::string ipaddress, bool req_stream, void* data_input, int data_size){
  stream = req_stream;
  _IPaddress_vect.push_back(ipaddress);
  makeclients({0});
  send(1, data_input, data_size, 0);
  void* buf = new char[data_size];
  void* idbuf = new char[5];
  if(!req_stream){
    int bytes = recv(1, idbuf, buf, data_size, 0);
  }
  close(0);
  delete[] (char*)buf;
  delete[] (char*)idbuf;
}

//server constructor
zeromq::zeromq(int port, bool rep_stream, void* buf, int data_size){
  stream = rep_stream;
  makeservers({port});
  void* idbuf = new char[5];
  while(1){
    int bytes = recv(0, idbuf, buf, data_size, 0);
    if(!rep_stream){
      send(0, buf, data_size, 0);
    }
  }
}

zeromq::~zeromq(){
}

std::string zeromq::int_to_str(int myValue){
  char value[10];
  sprintf(value,"%d",myValue);
  std::string value_str(value);
  return value;
}

//SEPARATE NEW FUNCTIONS

void zeromq::makeclients(std::vector<int> ipadd_locs){
  uint8_t loopsize = (uint8_t)ipadd_locs.size();
  for(uint8_t i_ipadd = 0; i_ipadd<loopsize; i_ipadd++){
    int ip_num = ipadd_locs[i_ipadd];
    std::string endpoint = "tcp://"+_IPaddress_vect[ip_num];
    printf ("Connecting to server at %s\n", endpoint.c_str());
    void* context = zmq_ctx_new ();
    void* requester = zmq_socket (context, (stream) ? ZMQ_STREAM : ZMQ_REP);
    zmq_connect (requester, &endpoint[0]);
    _Socket_vect.push_back(requester);
    _Context_vect.push_back(context);
  }
}


void zeromq::makeservers(std::vector<int> ports){
  void *context = zmq_ctx_new();
  void* responder;
  uint8_t loopsize = (uint8_t)ports.size();
  for(uint8_t i_port = 0; i_port<loopsize; i_port++){
    responder = zmq_socket(context, (stream) ? ZMQ_STREAM : ZMQ_REP);
    std::string port = int_to_str(ports[i_port]);
    //std::string port = IPaddress_vect[port_num].substr(IPaddress_vect[port_num].find(':') + 1);
    std::string endpoint = "tcp://*:"+port;
    zmq_bind(responder, &endpoint[0]);
    printf("Server bound to port %s\n", port.c_str());
    _Socketserver_vect.push_back(responder);
  }
}


void zeromq::send(bool client, void* input, int datasize, unsigned short whichConnection){
  uint32_t id[8];
  size_t sock_id_size = 8;
  int sock = zmq_getsockopt(_Socket_vect[whichConnection],ZMQ_ROUTING_ID, &id, &sock_id_size);
  void* socket = (client) ? _Socket_vect[whichConnection] : _Socketserver_vect[whichConnection];
  void* send_data = (stream) ? &id : input;
  int size = (stream) ? sock_id_size : datasize; 
  int bytes = zmq_send(socket, send_data, size, (stream) ? ZMQ_SNDMORE : 0);
  if(stream){
    int bytes_2 = zmq_send(socket, input, datasize, 0);
  }
  printf("Message sent\n");
}

int zeromq::recv(bool client, void*idbuf, void* buffer, int datasize, unsigned short whichConnection){
  //void* idbuf = new char[5];
  void* socket = (client) ? _Socket_vect[whichConnection] : _Socketserver_vect[whichConnection];
  int size = (stream) ? 5 : datasize;
  int bytes = zmq_recv(socket, (stream) ? idbuf : buffer, size, 0);
  printf("Received %d bytes...\n", bytes);
  int bytes_returned = bytes;
  if(stream){
    int bytes_2 = zmq_recv(socket, buffer, datasize, 0);
    printf("Received %d bytes...\n", bytes_2);
    bytes_returned = bytes_2;
  }
  //delete[] (char*)idbuf;
  //printf("end of recv\n");
  return bytes_returned;
}

/*
void zeromq::recv(bool client, void* buffer, int datasize, unsigned short whichConnection){
  void* idbuf = new char[5];
  void* socket = (client) ? _Socket_vect[whichConnection] : _Socketserver_vect[whichConnection];
  int size = (stream) ? 5 : datasize;
  int bytes = zmq_recv(socket, (stream) ? idbuf : buffer, size, 0);
  printf("Received %d bytes...\n", bytes);
  if(stream){
    int bytes_2 = zmq_recv(socket, buffer, datasize, 0);
    printf("Received %d bytes...\n", bytes_2);
  }
  delete[] (char*)idbuf;
  //printf("end of recv\n");
}
*/

//OLD FUNCTIONS
void zeromq::setconnectionID(uint32_t inputid){
  uint32_t _UserID = inputid;
}

//TEST CHANGING ID WHEN CONNECTING
void zeromq::connect_id(int enabledLoc){
  //_IPaddress_vect.push_back(ipaddress);
  if(enabledLoc > (int)_IPaddress_vect.size())
  {
    printf("No IP exists\n");
    return;
  }
  std::string endpoint = "tcp://"+_IPaddress_vect[enabledLoc];
  printf ("Connecting to server at %s\n", endpoint.c_str());
  void* context = zmq_ctx_new ();
  void* requester = zmq_socket (context, ZMQ_STREAM);
  //void* requester = zmq_socket (context, ZMQ_REQ);
  //zmq_connect (requester, &endpoint[0]);
  //void *p;
  //uint32_t id = 10;
  //userid = &id;
  size_t size = 4;
  int sock = zmq_setsockopt(requester, ZMQ_ROUTING_ID, &_UserID, size);
  uint32_t* id1[256];
  size_t sock_id_size = 256;
  int sock_get = zmq_getsockopt(requester,ZMQ_ROUTING_ID, id1, &sock_id_size);
  /*int *socketid = (int*)p;
  int value = *socketid;*/
  printf("Socket id set... %d, %p\n", _UserID, &_UserID); 
  printf("Socket id get... %d, %p\n", id1, &id1);
  zmq_connect (requester, &endpoint[0]);
  _Socket_vect.push_back(requester);
  _Context_vect.push_back(context);
}

void zeromq::connect(int enabledLoc, bool stream){
  //_IPaddress_vect.push_back(ipaddress);
  if(enabledLoc > (int)_IPaddress_vect.size())
  {
    printf("No IP exists\n");
    return;
  }
  std::string endpoint = "tcp://"+_IPaddress_vect[enabledLoc];
  printf ("Connecting to server at %s\n", endpoint.c_str());
  void* context = zmq_ctx_new ();
  void* requester = zmq_socket (context, (stream) ? ZMQ_STREAM : ZMQ_REP);
  //void* requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, &endpoint[0]);
  _Socket_vect.push_back(requester);
  _Context_vect.push_back(context);
}

void zeromq::clientconnect(int enabledLoc, bool stream){
  //_IPaddress_vect.push_back(ipaddress);
  if(enabledLoc > (int)_IPaddress_vect.size())
  {
    printf("No IP exists\n");
    return;
  }
  std::string endpoint = "tcp://"+_IPaddress_vect[enabledLoc];
  printf ("Connecting to server at %s\n", endpoint.c_str());
  void* context = zmq_ctx_new ();
  void* requester = zmq_socket (context, (stream) ? ZMQ_STREAM : ZMQ_REP);
  //void* requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, &endpoint[0]);
  _Socket_vect.push_back(requester);
  _Context_vect.push_back(context);
}


//TEST GETTING NEW ID WHEN SENDING
void zeromq::send_id(int socketnum, void* input, int data_size){
  uint32_t* id[8];
  size_t sock_id_size = 8;
  int sock = zmq_getsockopt(_Socket_vect[socketnum],ZMQ_ROUTING_ID, id, &sock_id_size);
  /*uint32_t id = 10;
  size_t size = 5;
  int sock = zmq_setsockopt(_Socket_vect[socketnum], ZMQ_ROUTING_ID, &id, size);
  */
  printf("Get socket id... %d\n", &id);
  zmq_send(_Socket_vect[socketnum], &_UserID, sizeof(_UserID), ZMQ_SNDMORE);
  printf("Client Sent ID... %p\n", &_UserID);
  printf("Socket ID... %d\n", _UserID); 
  zmq_send(_Socket_vect[socketnum], input, data_size, 0);
  printf("Client Sent Message... %p\n", input);
}

//USE THIS SEND FUNCTION
void zeromq::send_new(int socketnum, void* input, int data_size){
  uint32_t id[8];
  size_t sock_id_size = 8;
  int sock = zmq_getsockopt(_Socket_vect[socketnum],ZMQ_ROUTING_ID, &id, &sock_id_size);
  /*uint32_t id = 10;
  size_t size = 5;
  int sock = zmq_setsockopt(_Socket_vect[socketnum], ZMQ_ROUTING_ID, &id, size);
  */
  printf("Get socket id... %d\n", id);
  zmq_send(_Socket_vect[socketnum], &id, sock_id_size, ZMQ_SNDMORE);
  printf("Client Sent ID... %p\n", &id);
  printf("Socket ID... %d\n", id);
  zmq_send(_Socket_vect[socketnum], input, data_size, 0);
  printf("Client Sent Message... %p\n", input);
}

void zeromq::send_recv(int socketnum, void *input, void* output, int data_size){
  zmq_send(_Socket_vect[socketnum], input, data_size, 0);
  printf("Client Sent... %p\n", input);
  //int op_result = zmq_recv (_Socket_vect[socketnum], output, data_size, 0);
  //printf("Client Received...  number of bytes: %d\n", op_result);
}

void zeromq::close(int index){
  //std::string ipaddress_input;
  //std::cout<<"Enter the IP Address of the socket you wish to close\n";
  //std::cin>>ipaddress_input;
  //std::vector<std::string>::iterator itr = std::find(_IPaddress_vect.begin(), _IPaddress_vect.end(), ipaddress_input);
  //int index = itr - _IPaddress_vect.begin();
  zmq_close (_Socket_vect[index]);
  zmq_ctx_destroy (_Context_vect[index]);
  _Context_vect.erase(_Context_vect.begin()+index);
  _Socket_vect.erase(_Socket_vect.begin()+index);
  //std::remove(_IPaddress_vect.begin(), _IPaddress_vect.end(), ipaddress_input);
  //printf("Number of sockets remaining open: %d:\n", _Socket_vect.size());
}

void zeromq::connect_send_receive_close(void *input, void* output, int data_size, int num_data, std::string ipaddress){
  std::string endpoint = "tcp://"+ipaddress;
  printf ("Connecting to server…\n");
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, &endpoint[0]);

  printf ("Client Sending Request... \n");

  zmq_send(requester, input, data_size, 0);
  int op_result = zmq_recv (requester, output, data_size, 0);
  printf("Client Received...  number of bytes: %d\n", op_result);

  zmq_close (requester);
  zmq_ctx_destroy (context);
}

void zeromq::server(void* buffer, void* idbuf, std::vector<int> ports, int data_size, bool stream){
  void *context = zmq_ctx_new();
  void* responder;
  int loopsize = (int)ports.size();
  for(int i_port = 0; i_port<loopsize; i_port++){
    responder = zmq_socket(context, (stream) ? ZMQ_STREAM : ZMQ_REP);
    std::string port = int_to_str(ports[i_port]);
    std::string endpoint = "tcp://*:"+port;
    zmq_bind(responder, &endpoint[0]);
    printf("Server bound to port %s\n", port.c_str());
    _Socketserver_vect.push_back(responder);
  }
  while (1) {
    for(int i_port = 0; i_port<loopsize; i_port++){
      int nbytes = zmq_recv(_Socketserver_vect[i_port], (stream) ? idbuf : buffer, (stream) ? 5 : data_size, 0);
      if(stream){
        int nbytes_2 = zmq_recv(_Socketserver_vect[i_port], buffer, data_size, 0);
      /*if(stream == 0){
        int nbytes = zmq_recv (responder, buffer, data_size, 0);
        printf ("Server Received Message\n");
        printf ("bytes = %d\n", nbytes);
        zmq_send (responder, buffer, data_size, 0);
        printf ("Server Sent Message\n");
      }
      if(stream == 1){
        int nbytes_id = zmq_recv (responder, idbuf, 5, 0);
        printf ("Server Received ID\n");
        printf ("bytes = %d\n", nbytes_id);
        int nbytes_msg = zmq_recv (responder, buffer, data_size, 0);
        printf ("Server Received Message\n");
        printf ("bytes = %d\n", nbytes_msg);
    }*/
      }
    }
  }
}
