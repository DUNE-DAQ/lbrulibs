/**
 * @file ZMQIssues.hpp ND ZMQ related ERS issues
 *
 * This is part of the DUNE DAQ , copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef LBRULIBS_SRC_ZMQISSUES_HPP_
#define LBRULIBS_SRC_ZMQISSUES_HPP_

#include <ers/Issue.hpp>

#include <string>

namespace dunedaq {

ERS_DECLARE_ISSUE(lbrulibs, InitializationError, " ZMQ Initialization Error: " << initerror, ((std::string)initerror))

} // namespace dunedaq

#endif // LBRULIBS_SRC_ZMQISSUES_HPP_
