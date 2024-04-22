#ifndef LBRULIBS_SRC_HALISSUES_HPP_
#define LBRULIBS_SRC_HALISSUES_HPP_

#include <ers/Issue.hpp>

#include <string>

namespace dunedaq
{
  ERS_DECLARE_ISSUE(lbrulibs, InitializationError  , " HAL Initialization Error: " << initerror, ((std::string)initerror))

  ERS_DECLARE_ISSUE(lbrulibs, GenericNDMessage     , " ND Readout: "               << initmsg  , ((std::string)initmsg  ))

  ERS_DECLARE_ISSUE(lbrulibs, ReceiveTimeoutExpired, " Unable to receive within timeout period: " << timeout << " milliseconds.",((int)timeout))
}

#endif
