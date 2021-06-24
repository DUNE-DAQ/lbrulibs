#include <stdint.h>

#ifndef _LARPIX_H_
#define _LARPIX_H_

const uint32_t get_msg_bytes(void* msg);

enum msg_type { // message type declarations
    DATA_MSG = 0x44,
    REQ_MSG  = 0x3F,
    REP_MSG  = 0x21
};
uint8_t*  get_msg_type(void* msg);
uint16_t* get_msg_words(void* msg);
uint32_t* get_msg_unix_ts(void* msg);
void*     get_msg_word(void* msg, const uint32_t i);

enum word_type { // word type declarations
    DATA_WORD  = 0x44,
    TRIG_WORD  = 0x54,
    SYNC_WORD  = 0x53,
    PING_WORD  = 0x50,
    WRITE_WORD = 0x57,
    READ_WORD  = 0x52,
    TX_WORD    = 0x44,
    ERR_WORD   = 0x45
};
uint8_t*  get_word_type(void* word);
uint8_t*  get_word_io_channel(void* word);
uint32_t* get_word_receipt_timestamp(void* word);
uint64_t* get_word_packet(void* word);

enum packet_type { // packet type declarations
    DATA_PACKET         = 0x0,
    CONFIG_WRITE_PACKET = 0x2,
    CONFIG_READ_PACKET  = 0x3
};
const uint64_t get_packet_type(uint64_t* packet);
const uint64_t get_packet_chipid(uint64_t* packet);
const uint64_t get_packet_channelid(uint64_t* packet);
const uint64_t get_packet_timestamp(uint64_t* packet);
const uint64_t get_packet_first_packet(uint64_t* packet);
const uint64_t get_packet_dataword(uint64_t* packet);
const uint64_t get_packet_trigger_type(uint64_t* packet);
const uint64_t get_packet_local_fifo_status(uint64_t* packet);
const uint64_t get_packet_shared_fifo_status(uint64_t* packet);
const uint64_t get_packet_downstream_marker(uint64_t* packet);
const uint64_t get_packet_parity_bit(uint64_t* packet);

#endif
