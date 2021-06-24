#include <stdint.h>

#include "larpix.h"

/* ~~~ Access into message header and message contents ~~~ */

#define WORD_LEN   8  // bytes
#define HEADER_LEN 16 // bytes

const uint32_t get_msg_bytes(void* msg) {
    // get total number of bytes in message, including header
    return HEADER_LEN + WORD_LEN * (*get_msg_words(msg));
}

#define MSG_TYPE_OFFSET  0 // bytes
#define MSG_WORDS_OFFSET 6 // bytes
#define UNIX_TS_OFFSET   1 // bytes

uint8_t* get_msg_type(void* msg) {
    // get pointer to message type
    return (uint8_t*)(((uint8_t*)msg) + MSG_TYPE_OFFSET);
}

uint16_t* get_msg_words(void* msg) {
    // get pointer to number of message words
    return (uint16_t*)(((uint8_t*)msg) + MSG_WORDS_OFFSET);
}

uint32_t* get_msg_unix_ts(void* msg) {
    // get pointer to message unix timestamp
    return (uint32_t*)(((uint8_t*)msg) + UNIX_TS_OFFSET);
}

void* get_msg_word(void* msg, const uint32_t i) {
    // get pointer to word within message
    return (void*)(((uint8_t*)msg) + HEADER_LEN + WORD_LEN * i);
}


/* ~~~ Access into message words ~~~ */

#define WORD_TYPE_OFFSET         0 // bytes
#define IO_CHANNEL_OFFSET        1 // bytes
#define RECEIPT_TIMESTAMP_OFFSET 4 // bytes
#define PACKET_OFFSET            8 // bytes

uint8_t* get_word_type(void* word) {
    // get pointer to word type
    return (uint8_t*)(((uint8_t*)word) + WORD_TYPE_OFFSET);
}
uint8_t* get_word_io_channel(void* word) {
    // only valid for DATA type words, get pointer to the PACMAN io channel a word arrived on
    return (uint8_t*)(((uint8_t*)word) + IO_CHANNEL_OFFSET);
}
uint32_t* get_word_receipt_timestamp(void* word) {
    // only valid for DATA type words, get pointer to the timestamp when word was received at the PACMAN
    return (uint32_t*)(((uint8_t*)word) + RECEIPT_TIMESTAMP_OFFSET);
}
uint64_t* get_word_packet(void* word) {
    // only valid for DATA type words, get pointer to the LArPix word
    return (uint64_t*)(((uint8_t*)word) + PACKET_OFFSET);
}


/* ~~~ Access into LArPix packets ~~~ */

const uint64_t get_packet_data(uint64_t* packet, const uint8_t bit_offset, const uint64_t bit_mask) {
    // right-shift packet bits by bit_offset and then grab bits specified by bit_mask
    return (*packet >> bit_offset) & bit_mask;
}

#define PACKET_TYPE_OFFSET 0   // bits
#define PACKET_TYPE_MASK   0x3 // bitmask
const uint64_t get_packet_type(uint64_t* packet) {
   // bits [0:1]
   return get_packet_data(packet, PACKET_TYPE_OFFSET, PACKET_TYPE_MASK);
}

#define PACKET_CHIPID_OFFSET 8    // bits
#define PACKET_CHIPID_MASK   0xFF // bitmask
const uint64_t get_packet_chipid(uint64_t* packet) {
    // bits [2:9]
    return get_packet_data(packet, PACKET_CHIPID_OFFSET, PACKET_CHIPID_MASK);
}

#define PACKET_CHANNELID_OFFSET 10   // bits
#define PACKET_CHANNELID_MASK   0x3F // bitmask
const uint64_t get_packet_channelid(uint64_t* packet) {
    // bits [10:15], only valid for data packets
    return get_packet_data(packet, PACKET_CHANNELID_OFFSET, PACKET_CHANNELID_MASK);
}

#define PACKET_TIMESTAMP_OFFSET 16         // bits
#define PACKET_TIMESTAMP_MASK   0x7FFFFFFF // bitmask
const uint64_t get_packet_timestamp(uint64_t* packet) {
    // bits [46:16], only valid for data packets
    return get_packet_data(packet, PACKET_TIMESTAMP_OFFSET, PACKET_TIMESTAMP_MASK);
}

#define PACKET_FIRST_PACKET_OFFSET 47  // bits
#define PACKET_FIRST_PACKET_MASK   0x1 // bitmask
const uint64_t get_packet_first_packet(uint64_t* packet) {
    // bits [47], only valid for data packets
    return get_packet_data(packet, PACKET_FIRST_PACKET_OFFSET, PACKET_FIRST_PACKET_MASK);
}

#define PACKET_DATAWORD_OFFSET 48   // bits
#define PACKET_DATAWORD_MASK   0xFF // bitmask
const uint64_t get_packet_dataword(uint64_t* packet) {
    // bits [48:55], only valid for data packets
    return get_packet_data(packet, PACKET_DATAWORD_OFFSET, PACKET_DATAWORD_MASK);
}

#define PACKET_TRIGGER_TYPE_OFFSET 56  // bits
#define PACKET_TRIGGER_TYPE_MASK  0x3 // bitmask
const uint64_t get_packet_trigger_type(uint64_t* packet) {
    // bits [56:57], only valid for data packets
    return get_packet_data(packet, PACKET_TRIGGER_TYPE_OFFSET, PACKET_TRIGGER_TYPE_MASK);
}

#define PACKET_LOCAL_FIFO_STATUS_OFFSET 58  // bits
#define PACKET_LOCAL_FIFO_STATUS_MASK   0x3 // bitmask
const uint64_t get_packet_local_fifo_status(uint64_t* packet) {
    // bits [58:59], only valid for data packets
    return get_packet_data(packet, PACKET_LOCAL_FIFO_STATUS_OFFSET, PACKET_LOCAL_FIFO_STATUS_MASK);
}

#define PACKET_SHARED_FIFO_STATUS_OFFSET 60  // bits
#define PACKET_SHARED_FIFO_STATUS_MASK   0x3 // bitmask
const uint64_t get_packet_shared_fifo_status(uint64_t* packet) {
    // bits [60:61], only valid for data packets
    return get_packet_data(packet, PACKET_SHARED_FIFO_STATUS_OFFSET, PACKET_SHARED_FIFO_STATUS_MASK);
}

#define PACKET_DOWNSTREAM_MARKER_OFFSET 62  // bits
#define PACKET_DOWNSTREAM_MARKER_MASK   0x1 // bitmask
const uint64_t get_packet_downstream_marker(uint64_t* packet) {
    // bits [62], only valid for data packets
    return get_packet_data(packet, PACKET_DOWNSTREAM_MARKER_OFFSET, PACKET_DOWNSTREAM_MARKER_MASK);
}

#define PACKET_PARITY_BIT_MARKER_OFFSET 63  // bits
#define PACKET_PARITY_BIT_MARKER_MASK   0x1 // bitmask
const uint64_t get_packet_parity_bit(uint64_t* packet) {
    // bits [63], only valid for data packets
    return get_packet_data(packet, PACKET_PARITY_BIT_MARKER_OFFSET, PACKET_PARITY_BIT_MARKER_MASK);
}
