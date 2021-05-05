/**
* @file AvailableParserOperations.hpp Implementations of parsers into user types.
*
* This is part of the DUNE DAQ , copyright 2021.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef LBRULIBS_INCLUDE_LBRULIBS_AVAILABLEPARSEROPERATIONS_HPP_
#define LBRULIBS_INCLUDE_LBRULIBS_AVAILABLEPARSEROPERATIONS_HPP_

#include "appfwk/DAQSink.hpp"
#include "readout/ReadoutTypes.hpp"


#include <sstream>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <utility>

namespace dunedaq {
namespace parsers {

inline void 
print_bytes(std::ostream& ostr, const char *title, const unsigned char *data, std::size_t length, bool format = true) {
  ostr << title << std::endl;
  ostr << std::setfill('0');
  for(size_t i = 0; i < length; ++i) {
    ostr << std::hex << std::setw(2) << static_cast<int>(data[i]);
    if (format) {
      ostr << (((i + 1) % 16 == 0) ? "\n" : " ");
    }
  }
  ostr << std::endl;
}

inline void
dump_to_buffer(const char* data, std::size_t size,
               void* buffer, uint32_t buffer_pos, // NOLINT
               const std::size_t& buffer_size)
{
  auto bytes_to_copy = size; // NOLINT
  while(bytes_to_copy > 0) {
    auto n = std::min(bytes_to_copy, buffer_size-buffer_pos); // NOLINT
    std::memcpy(static_cast<char*>(buffer)+buffer_pos, data, n);
    buffer_pos += n;
    bytes_to_copy -= n;
    if(buffer_pos == buffer_size) {
      buffer_pos = 0;
    }
  }
}

inline std::function<void(const data)>
varsizedChunkIntoWrapper(std::unique_ptr<appfwk::DAQSink<readout::types::VariableSizePayloadWrapper>>& sink,
                         std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const data) {
    auto data_length = sizeof(data);

    char* payload = static_cast<char*>( malloc(data_length * sizeof(char)) );
    uint32_t bytes_copied = 0;
    dump_to_buffer(data, data_length,
                    static_cast<void*>(payload),
                    bytes_copied,
                    data_length);
    }
    readout::types::VariableSizePayloadWrapper payload_wrapper(data_length, payload);
    try {
      sink->push(std::move(payload_wrapper), timeout);
    } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
      // ers 
    }
  };
}

} // namespace parsers
} // namespace dunedaq

#endif // LBRULIBS_INCLUDE_LBRULIBS_AVAILABLEPARSEROPERATIONS_HPP_
