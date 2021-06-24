/**
 * @file NDReadoutTypes.hpp Payload type structures for the DUNE Near Detector
 *
 * This is part of the DUNE DAQ , copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef LBRULIBS_SRC_NDREADOUTTYPES_HPP_
#define LBRULIBS_SRC_NDREADOUTTYPES_HPP_

#include "appfwk/DAQSink.hpp"
#include "appfwk/DAQSource.hpp"

#include "dataformats/FragmentHeader.hpp"
#include "dataformats/GeoID.hpp"
#include "PACMANFrame.hpp"

#include <cstdint> // uint_t types
#include <memory>  // unique_ptr

namespace dunedaq {
namespace readout {
namespace types {

/**
 * @brief PACMAN frame
 * Size = 816[Bytes] (12*64+1*32+2*8)
 * */
const constexpr std::size_t PACMAN_FRAME_SIZE = 816; // FIX ME - check this
struct PACMAN_MESSAGE_STRUCT
{
  // data
  char data[PACMAN_FRAME_SIZE];
  // comparable based on first timestamp
  bool operator<(const PACMAN_MESSAGE_STRUCT& other) const
  {
    auto thisptr = reinterpret_cast<const PACMANFrame*>(&data);        // NOLINT
    auto otherptr = reinterpret_cast<const PACMANFrame*>(&other.data); // NOLINT
    return thisptr->get_timestamp() < otherptr->get_timestamp() ? true : false;
  }

  uint64_t get_timestamp() const // NOLINT(build/unsigned)
  {
    return reinterpret_cast<const PACMANFrame*>(&data)->get_msg_unix_ts(); // NOLINT
  }

  void set_timestamp(uint64_t ts) // NOLINT(build/unsigned)
  {
    auto frame = reinterpret_cast<PACMANFrame*>(&data); // NOLINT
    frame->header.msg_unix_timestamp = ts;
  }

  // FIX ME - figure out what this is and what to do for ND
  //static const constexpr dataformats::GeoID::SystemType system_type = dataformats::GeoID::SystemType::kTPC;
  //static const constexpr dataformats::FragmentType fragment_type = dataformats::FragmentType::kTPCData;
};

typedef dunedaq::appfwk::DAQSink<PACMAN_MESSAGE_STRUCT> PACMANFrameSink;
typedef std::unique_ptr<PACMANFrameSink> UniquePACMANFrameSink;
using PACMANFramePtrSink = appfwk::DAQSink<std::unique_ptr<types::PACMAN_MESSAGE_STRUCT>>;
using UniquePACMANFramePtrSink = std::unique_ptr<PACMANFramePtrSink>;

} // namespace types
} // namespace readout
} // namespace dunedaq

#endif // LBRULIBS_SRC_NDREADOUTTYPES_HPP_