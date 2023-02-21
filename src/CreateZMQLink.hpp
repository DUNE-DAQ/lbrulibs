/**
* @file CreateZMQLink.hpp Specific ElinkConcept creator.
*
* This is part of the DUNE DAQ , copyright 2021.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef LBRULIBS_SRC_CREATEZMQLINK_HPP_
#define LBRULIBS_SRC_CREATEZMQLINK_HPP_

#include "ndreadoutlibs/NDReadoutPACMANTypeAdapter.hpp"
#include "ndreadoutlibs/NDReadoutMPDTypeAdapter.hpp"
#include "ZMQLinkModel.hpp"

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
DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::PACMAN_MESSAGE_STRUCT, "PACMANFrame")
DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::MPD_MESSAGE_STRUCT, "MPDFrame")
#endif // LBRULIBS_SRC_DEFINE_TYPESTRINGS_

namespace lbrulibs {

std::unique_ptr<ZMQLinkConcept>
createZMQLinkModel(const std::string& target)
{   
  if (target.find("pacman") != std::string::npos) {

    ers::info(GenericNDMessage(ERS_HERE, "CreateZMQLinkModel Creating Link for Pacman!"));

    // Create Model
    auto zmqlink_model = std::make_unique<ZMQLinkModel<ndreadoutlibs::types::PACMAN_MESSAGE_STRUCT>>();
    // Setup sink (acquire pointer from QueueRegistry)
    zmqlink_model->set_sink(target);
    // Get sink
    //auto& sink = zmqlink_model->get_sink();

    // Return with setup model
    return zmqlink_model;

  } else if (target.find("mpd") != std::string::npos) {

    ers::info(GenericNDMessage(ERS_HERE, "CreateZMQLinkModel Creating Link for MPD!"));

    // Create Model
    auto zmqlink_model = std::make_unique<ZMQLinkModel<ndreadoutlibs::types::MPD_MESSAGE_STRUCT>>();

    zmqlink_model->set_sink(target);

    return zmqlink_model;
  }

  ers::warning(GenericNDMessage(ERS_HERE, "CreateZMQLinkModel Could not find target!"));

  return nullptr;
}

} // namespace lbrulibs
} // namespace dunedaq

#endif // LBRULIBS_SRC_CREATEZMQLINK_HPP_
