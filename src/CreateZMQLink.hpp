/**
* @file CreateZMQLink.hpp Specific ElinkConcept creator.
*
* This is part of the DUNE DAQ , copyright 2021.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef LBRULIBS_SRC_CREATEZMQLINK_HPP_
#define LBRULIBS_SRC_CREATEZMQLINK_HPP_

#include "readout/ReadoutTypes.hpp"
#include "ZMQLinkModel.hpp"

#include <memory>
#include <string>

namespace dunedaq {
namespace lbrulibs {

createZMQLinkModel(const std::string& target)
{   
  if (target.find("varsize") != std::string::npos) {
    // Create Model
    auto zmqlink_model = std::make_unique<ZMQLinkModel<readout::types::VariableSizePayloadWrapper>>();

    // Setup sink (acquire pointer from QueueRegistry)
    zmqlink_model->set_sink(target);

    // Get parser and sink
    auto& parser = zmqlink_model->get_parser();
    auto& sink = zmqlink_model->get_sink();

    // Return with setup model
    return zmqlink_model;
  }

  return nullptr;
}

} // namespace lbrulibs
} // namespace dunedaq

#endif // LBRULIBS_SRC_CREATEELINK_HPP_
