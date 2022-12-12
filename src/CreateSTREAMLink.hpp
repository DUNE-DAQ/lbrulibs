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
