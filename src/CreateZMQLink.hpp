/**
* @file CreateZMQLink.hpp Specific ElinkConcept creator.
*
* This is part of the DUNE DAQ , copyright 2021.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef LBRULIBS_SRC_CREATEZMQLINK_HPP_
#define LBRULIBS_SRC_CREATEZMQLINK_HPP_

#include "readout/NDReadoutTypes.hpp"
#include "ZMQLinkModel.hpp"

#include "ZMQIssues.hpp"

#include <memory>
#include <string>

namespace dunedaq {
namespace lbrulibs {

std::unique_ptr<ZMQLinkConcept>
createZMQLinkModel(const std::string& target)
{   
  if (target.find("pacman") != std::string::npos) {

    ers::info(GenericNDMessage(ERS_HERE, "CreateZMQLinkModel Creating Link for Pacman!"));

    // Create Model
    auto zmqlink_model = std::make_unique<ZMQLinkModel<readout::types::PACMAN_MESSAGE_STRUCT>>();

    // Setup sink (acquire pointer from QueueRegistry)
    zmqlink_model->set_sink(target);

    // Get sink
    //auto& sink = zmqlink_model->get_sink();

    // Return with setup model
    return zmqlink_model;
  }

  ers::warning(GenericNDMessage(ERS_HERE, "CreateZMQLinkModel Could not find target!"));

  return nullptr;
}

} // namespace lbrulibs
} // namespace dunedaq

#endif // LBRULIBS_SRC_CREATEELINK_HPP_
