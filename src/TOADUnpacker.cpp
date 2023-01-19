#include "lbrulibs/TOADUnpacker.h"
#include <cstdio>
#include <iostream>
#include <fstream>

TOADUnpacker::TOADUnpacker() {
  output_vect = {};
}

TOADUnpacker::~TOADUnpacker(){
}

uint64_t TOADUnpacker::bitmask(uint64_t value, uint64_t n, uint64_t m){
  uint64_t mask = (((1ULL<<n)-1ULL) & (value >> m));
  return mask;
}

unsigned char TOADUnpacker::read_id(uint64_t word){
  unsigned char word_id;
  word_id = bitmask(word, 1ULL, 63ULL);
  return word_id;
}

void TOADUnpacker::get_timestamp(){
  timestamp = bitmask(header_in, 32ULL, 24ULL);
}

void TOADUnpacker::read_header(){
  fec_num = bitmask(header_in, 9ULL, 0ULL);
  outpt.fec = fec_num;
  num_clusters = bitmask(header_in, 10ULL, 9ULL);
  channel_id = bitmask(header_in, 5ULL, 19ULL);
  outpt.channel = channel_id;
  timestamp = bitmask(header_in, 32ULL, 24ULL);
  outpt.tstmp = timestamp;
  hdr_par = bitmask(header_in, 1ULL, 56ULL);
  data_par = bitmask(header_in, 1ULL, 57ULL);
  padd_words = bitmask(header_in, 3ULL, 58ULL);
  res = bitmask(header_in, 2ULL, 61ULL);
  hdr_id = bitmask(header_in, 1ULL, 63ULL);
}

void TOADUnpacker::read_data(){
  words[0] = bitmask(data_in, 10ULL, 0ULL);
  words[1] = bitmask(data_in, 10ULL, 10ULL);
  words[2] = bitmask(data_in, 10ULL, 20ULL);
  words[3] = bitmask(data_in, 10ULL, 30ULL);
  words[4] = bitmask(data_in, 10ULL, 40ULL);
  words[5] = bitmask(data_in, 10ULL, 50ULL); 
  padd = bitmask(data_in, 3ULL, 60ULL);
  data_id = bitmask(data_in, 1ULL, 63ULL);
}

std::vector<dunedaq::detdataformats::toad::TOADFrame> TOADUnpacker::decode_deque(std::deque<uint8_t>& buffer){ //UNPACKS DATA FROM A DEQUE WHERE SOCKET ID IS NOT IN DEQUE
  printf("begin decoding...\n");
  uint64_t word, word_hdr, word_data;
  int data_packet_limit, deque_sz_limit, bytes_left, num_data_packets, incrementer, incrementer1, prev_byte_num;
  int count = 0;
  incrementer1 = 0;
  incrementer = 0;
  int deque_sz = buffer.size();
  for(int k = 0; k<deque_sz; k+= incrementer){ //k indicates initial position of each header in deque 
    int elmnt_frst_wrd = 7;
    word = buffer[elmnt_frst_wrd];
    for(int j=0; j<7; j++){
      word = ((word<<8) | buffer[elmnt_frst_wrd - 1-j]); //little endian
    }
    unsigned char word_id = read_id(word);
    int prev_outpt_sz = output_vect.size();
    if(word_id == 1){ // Check if it's a header
      header_in = (uint64_t)word;
      read_header();
      //printf("header vals: %u, %d, %d, %d, %d, %d, %d, %d, %d\n", fec_num, (int)num_clusters, (int)channel_id, (int)timestamp, (int)hdr_par, (int)data_par, (int)padd_words, (int)res, (int)hdr_id);
      num_data_packets = (num_clusters + 5)/6;
      data_packet_limit = 8*(num_data_packets+1); //+1 for the header
      deque_sz_limit = deque_sz - k; //number of elements left in deque
      bytes_left = deque_sz_limit - data_packet_limit;
      if(bytes_left < 0){ //check if data packets are split over multiple receive. i.e. if we must wait for the next payload before unpacking
        incrementer = 0;
        break;
      }
      for(int i=8; i<data_packet_limit; i+=8){
        word_data = buffer[i+elmnt_frst_wrd];
        for(int j =0; j<7; j++){
          word_data = (word_data<<8) | buffer[i+elmnt_frst_wrd-1-j];
        }
        data_in = word_data;
        read_data();
        for(int x = 0; x<(6-padd); x++){
          outpt.toadsamples.push_back(words[x]);
          //output_vect.push_back(outpt);
          //outpt.toadsamples.clear();
	}
      }
      output_vect.push_back(outpt);
      outpt.toadsamples.clear();
      for(int i=0; i<data_packet_limit; i++){
        buffer.pop_front(); //remove elements from deque
      }
    }
    //printf("sizes %d, %d, %d\n", deque_sz, buffer.size(), k);
    if(word_id != 1){
      printf("no header... error\n");
    }
    /*int output_vect_diff = output_vect.size() - prev_outpt_sz; //number of new samples unpacked
    if(output_vect_diff != num_clusters){
      printf("problem ");
    }*/
    incrementer = data_packet_limit;
    incrementer_count = k;
  }
  printf("number of new decoded data structs: %d\n", output_vect.size());
  return output_vect;
}

