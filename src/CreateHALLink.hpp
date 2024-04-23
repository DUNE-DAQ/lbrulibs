#ifndef LBRULIBS_SRC_CREATEHALLINK_HPP_
#define LBRULIBS_SRC_CREATEHALLINK_HPP_

#include "ndreadoutlibs/NDReadoutPATTypeAdapter.hpp"
#include "HALLinkModel.hpp"

#include "HALIssues.hpp"

#include <memory>
#include <string>

namespace dunedaq
{

#ifndef LBRULIBS_SRC_DEFINE_TYPESTRINGS_
#define LBRULIBS_SRC_DEFINE_TYPESTRINGS_
  DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::NDReadoutPATTypeAdapter, "PATFrame");
#endif

  namespace lbrulibs
  {
    std::unique_ptr<HALLinkConcept>
    createHALLinkModel(const std::string &target)
    {
      if (target.find("pat") != std::string::npos)
	{
	  ers::info(GenericNDMessage(ERS_HERE, "CreateHALLinkModel Creating Link for PAT!"));
	  auto hallink_model = std::make_unique<HALLinkModel<ndreadoutlibs::types::NDReadoutPATTypeAdapter>>();
	  printf("HERE!!!!!1\n");
	  hallink_model->set_sink(target);
	  return hallink_model;
	}
      ers::warning(GenericNDMessage(ERS_HERE, "CreateHALLinkModel Could not find target!"));

      return nullptr;
    }
  }
}

#endif
