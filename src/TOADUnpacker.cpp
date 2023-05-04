#include "lbrulibs/TOADUnpacker.h"
#include "ZMQIssues.hpp"
#include <ers/Issue.hpp>
#include <cstdio>
#include <iostream>
#include <fstream>

namespace dunedaq {
namespace lbrulibs {
namespace toadunpacker {
TOADUnpacker::TOADUnpacker() {
  output_vect = {};
}

TOADUnpacker::~TOADUnpacker(){
}

//BITMASK FUNCTION
uint64_t TOADUnpacker::bitmask(uint64_t value, uint64_t n, uint64_t m){
  uint64_t mask = (((1ULL<<n)-1ULL) & (value >> m));
  return mask;
}

//READ ID OF PACKET - defs hdr or data packet
unsigned char TOADUnpacker::read_id(uint64_t word){
  unsigned char word_id;
  word_id = bitmask(word, 1ULL, 63ULL);
  return word_id;
}

//CALCULATE FULL TIMESTAMP
void TOADUnpacker::get_timestamp(){
  printf("timestamp_bs: %lu, timewindow: %d, timesample: %d", timestamp_bs, (uint32_t)timewindow, (uint32_t)timesample);
  outpt.tstmp = (timestamp_bs + timewindow*sampa_ticks*sampa_freq + timesample*sampa_freq); //HARDCODED FROM TICKS TO TS
}

//UNPACK HEADER
void TOADUnpacker::read_header(){
  res = bitmask(header_in, 6ULL, 0ULL);
  hdr_par = bitmask(header_in, 1ULL, 6ULL);
  timesample = bitmask(header_in, 10ULL, 7ULL);
  timewindow = bitmask(header_in, 20ULL, 17ULL);
  num_clusters = bitmask(header_in, 10ULL, 48ULL);
  fec_num = (bitmask(header_in, 5ULL, 58ULL));
  printf("fec %d", (int)fec_num);
  fec_num = (fec_num << 11) | (bitmask(header_in, 11ULL, 37ULL));
  printf("fec %d", (int)fec_num);
  outpt.fec = fec_num;
  hdr_id = bitmask(header_in, 1ULL, 63ULL);
}

//UNPACK DATA
void TOADUnpacker::read_data(){
  words[5] = bitmask(data_in, 10ULL, 0ULL);
  words[4] = bitmask(data_in, 10ULL, 10ULL);
  words[3] = bitmask(data_in, 10ULL, 20ULL);
  words[2] = bitmask(data_in, 10ULL, 30ULL);
  words[1] = bitmask(data_in, 10ULL, 40ULL);
  words[0] = bitmask(data_in, 10ULL, 50ULL); 
  padd = bitmask(data_in, 3ULL, 60ULL);
  data_id = bitmask(data_in, 1ULL, 63ULL);
}

//CALCULATE HEADER PARITY AND COMPARE WITH PARITY WHEN HEADER WAS SENT
void TOADUnpacker::calc_hdr_parity() {
  uint64_t y;
  bool hdr_par_recv;
  y = header_in;
  y = y ^ (y >> 32);
  y = y ^ (y >> 16);
  y = y ^ (y >> 8);
  y = y ^ (y >> 4);
  y = y ^ (y >> 2);
  y = y ^ (y >> 1);
  hdr_par_recv = (y & 1);
  if(hdr_par_recv == hdr_par){
    outpt.hdr_par_check = 1;
  }
  else if(hdr_par_recv != hdr_par){
    outpt.hdr_par_check = 0;
  }
}

std::vector<dunedaq::detdataformats::toad::TOADFrame> TOADUnpacker::decode_deque(std::deque<uint8_t>& buffer){ //UNPACKS DATA FROM A DEQUE WHERE SOCKET ID IS NOT IN DEQUE
  printf("begin decoding...\n");
  uint64_t word, word_hdr, word_data, word_ts;
  int data_packet_limit, deque_sz_limit, bytes_left, num_data_packets, incrementer, incrementer1, prev_byte_num;
  int count = 0;
  incrementer1 = 0;
  incrementer = 0;
  int deque_sz = buffer.size();
  int vect_samp_sz = 0;
  for(int k = 0; k<deque_sz; k+= incrementer){ //k indicates initial position of each header in deque 
    int elmnt_frst_wrd = 0;
    word = buffer[elmnt_frst_wrd];
    for(int j=0; j<7; j++){
      word = ((word<<8) | buffer[elmnt_frst_wrd + 1 +j]); //big endian - combine 8 consecutive bytes to form a full 64 bit packet
    }
    unsigned char word_id = read_id(word);
    int prev_outpt_sz = output_vect.size();
    int prev_samp_sz = vect_samp_sz;
    if(word_id == 1){ // Check if it's a header
      header_in = (uint64_t)word;
      read_header();
      calc_hdr_parity(); //check header parity
      //printf("header vals: %u, %d, %d, %d, %d, %d, %d, %d, %d\n", fec_num, (int)num_clusters, (int)channel_id, (int)timestamp, (int)hdr_par, (int)data_par, (int)padd_words, (int)res, (int)hdr_id);
      num_data_packets = (num_clusters + 5)/6;
      data_packet_limit = 8*(num_data_packets+2); //+2 for the header+timestamp
      deque_sz_limit = deque_sz - k; //number of elements left in deque
      bytes_left = deque_sz_limit - data_packet_limit;
      if(bytes_left < 0){ //check if data packets are split over multiple receive. i.e. if we must wait for the next payload before unpacking
        incrementer = 0;
        ers::warning(dunedaq::lbrulibs::UnpackingError(ERS_HERE, "Data packets spilt, waiting for more to arrive"));
        break;
      }
      word_ts = buffer[8+elmnt_frst_wrd];
      for(int j =0; j<7; j++){
        word_ts = (word_ts<<8) | buffer[elmnt_frst_wrd+8+1+j]; //big endian
      }
      timestamp_bs = word_ts;
      get_timestamp();
      for(int i=16; i<data_packet_limit; i+=8){ //start at 16 bc of hdr and timestamp bytes
        word_data = buffer[i+elmnt_frst_wrd];
        for(int j =0; j<7; j++){
          word_data = (word_data<<8) | buffer[i+elmnt_frst_wrd+1+j]; //big endian
        }
        data_in = word_data;
        read_data();
        for(int x = 0; x<(6-padd); x++){
          outpt.toadsamples.push_back(words[x]);
	}
      }
      vect_samp_sz = outpt.toadsamples.size();
      output_vect.push_back(outpt);
      outpt.toadsamples.clear();
      for(int i=0; i<data_packet_limit; i++){
        buffer.pop_front(); //remove elements from deque that have been unpacked
      }
    }
    //printf("sizes %d, %d, %d\n", deque_sz, buffer.size(), k);
    if(word_id != 1){
      ers::fatal(dunedaq::lbrulibs::UnpackingError(ERS_HERE, "No Header Found... TOADUnpacker cannot decode data stream"));
      printf("no header... error\n"); //if id is not a header then something has gone wrong when unpacking
    }
    //int output_vect_diff = output_vect.size() - prev_outpt_sz; //number of new samples unpacked
    if(vect_samp_sz != num_clusters){
      ers::warning(dunedaq::lbrulibs::UnpackingError(ERS_HERE, "Number of new decoded data structs in TOADUnpacker does not match number of clusters"));
      //printf("problem ");
    }
    incrementer = data_packet_limit;
    incrementer_count = k;
  }
  printf("number of new decoded data structs: %d\n", output_vect.size());
  return output_vect;
}
} //namespace toadunpacker
} //namespace lbrulibs
} //namespace dunedaq
