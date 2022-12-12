/**
 * * @file CreateSTREAMLink.hpp Specific ElinkConcept creator.
 * *
 * * This is part of the DUNE DAQ , copyright 2022.
 * * Licensing/copyright details are in the COPYING file that you should have
 * * received with this code.
 * */
#ifndef LBRULIBS_SRC_CREATESTREAMLINK_HPP_
#define LBRULIBS_SRC_CREATESTREAMLINK_HPP_

#include "STREAMLinkModel.hpp"
#include "ndreadoutlibs/NDReadoutTypes.hpp"

#include "ZMQIssues.hpp"

#include <memory>
#include <string>

namespace dunedaq {

// 09-Dec-2022, KAB: Added the declaration of the message type string for the
// PACMAN message struct, needed for serialization of this struct when sending
// or receiving it over the network with the IOManager ConnectivityService changes.
// In this header file, this is done in a way that support its declaration in other
// header files in this package (because this level of safety is needed).
#ifndef LBRULIBS_SRC_DEFINE_TYPESTRINGS_
#define LBRULIBS_SRC_DEFINE_TYPESTRINGS_
DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::PACMAN_MESSAGE_STRUCT, "PACMAN")
#endif // LBRULIBS_SRC_DEFINE_TYPESTRINGS_

namespace lbrulibs {

std::unique_ptr<STREAMLinkConcept>
createSTREAMLinkModel(const std::string& target)
{
  if (target.find("pacman") != std::string::npos) {

    ers::info(GenericNDMessage(ERS_HERE, "CreateSTREAMLinkModel Creating Link for Pacman!"));

    // Create Model
    auto streamlink_model = std::make_unique<STREAMLinkModel<ndreadoutlibs::types::PACMAN_MESSAGE_STRUCT>>();
    // Setup sink (acquire pointer from QueueRegistry)
    streamlink_model->set_sink(target);
    return streamlink_model;
  }

  ers::warning(GenericNDMessage(ERS_HERE, "CreateSTREAMLinkModel Could not find target!"));

  return nullptr;
}

} // namespace lbrulibs
} // namespace dunedaq

#endif // LBRULIBS_SRC_CREATESTREAMLINK_HPP_
