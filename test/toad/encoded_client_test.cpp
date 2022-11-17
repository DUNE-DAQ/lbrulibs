#include "librulibs/zeromq.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include<string>
#include<vector>

int main(int argc, char *argv[]){
  std::string filename = argv[1];
  std::ifstream file(filename.c_str(), std::ifstream::in);
  if(!file){
    printf("Cannot Open File");
    return 0;
  }
  int lines_count = 0;
  std::string str;
  std::vector<uint8_t> buf_vect;
  while (std::getline(file, str)){
    if(str.size() > 0){
      //printf(str.c_str());
      int ints = std::stoi(str);
      //printf("%d\n", ints);
      uint8_t byte_int = (uint8_t)ints;
      buf_vect.push_back(byte_int);
      //printf("%d\n", buf_vect[lines_count]);
      ++lines_count;
      //printf("%d\n", byte_int);
      //printf("%d\n", lines_count);
    }
  }
  uint8_t buffer[lines_count];
  printf("buffer size = %d\n", sizeof(buffer));
  for(int i=0; i<lines_count; i++){
    //printf("%d\n", buf_vect[i]);
    buffer[i] = buf_vect[i];
    //printf("%d ,", (int)buffer[i]);
  }
  buf_vect.clear();
  std::string ipadd = "localhost:5556";
  int datasize = sizeof(buffer);
  zeromq Client(ipadd, 1, &buffer, datasize); 
  return 0;
}
