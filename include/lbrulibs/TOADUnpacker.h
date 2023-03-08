#ifndef LBRULIBS_INCLUDE_LBRULIBS_TOADUNPACKER_HPP
#define LBRULIBS_INCLUDE_LBRULIBS_TOADUNPACKER_HPP
#include <cstdint>
#include <vector>
#include <string>
#include <deque>
#include "detdataformats/toad/TOADFrame.hpp"

class TOADUnpacker {
  public:
    uint64_t header_in;
    uint64_t data_in;

    unsigned fec_num: 16;
    unsigned num_clusters: 10;
    //unsigned channel_id: 5;
    uint64_t timestamp: 64;
    unsigned timesample: 10;
    unsigned timewindow: 20;
    uint64_t timestamp_bs: 64;
    bool hdr_par = 0;
    unsigned padd_words: 3;
    unsigned res: 2;
    unsigned hdr_id: 1;

    int words [6];
    unsigned padd: 3;
    unsigned data_id: 1;

    dunedaq::detdataformats::toad::TOADFrame outpt;
    std::vector<dunedaq::detdataformats::toad::TOADFrame> output_vect;
    int incrementer_count;

    TOADUnpacker();
    ~TOADUnpacker();
    uint64_t bitmask(uint64_t value, uint64_t n, uint64_t m);
    unsigned char read_id(uint64_t word);
    void get_timestamp();
    void read_header();
    void read_data();
    void calc_hdr_parity();
    std::vector<dunedaq::detdataformats::toad::TOADFrame> decode_deque(std::deque<uint8_t>& buffer);
  private:
    int accelerator_clock_freq = 56000000;
    int sampa_ticks = 1024;
    int sampa_freq = 20000000;
};


/*
class TOADUnpacker {
  public:
    uint64_t header_in;
    uint64_t data_in;

    unsigned fec_num: 9;
    unsigned num_clusters: 10;
    unsigned channel_id: 5;
    unsigned timestamp: 32;
    bool hdr_par;
    bool data_par;
    unsigned padd_words: 3;
    unsigned res: 2;
    unsigned hdr_id: 1;

    int words [6];
    unsigned padd: 3;
    unsigned data_id: 1;

    dunedaq::detdataformats::toad::TOADFrame outpt;
    std::vector<dunedaq::detdataformats::toad::TOADFrame> output_vect;
    int incrementer_count;

    TOADUnpacker();
    ~TOADUnpacker();
    uint64_t bitmask(uint64_t value, uint64_t n, uint64_t m);
    unsigned char read_id(uint64_t word);
    void get_timestamp();
    void read_header();
    void read_data();
    std::vector<dunedaq::detdataformats::toad::TOADFrame> decode_deque(std::deque<uint8_t>& buffer);
};
*/
#endif

