#ifndef LBRULIBS_PLUGINS_PATCARDREADER_HPP_
#define LBRULIBS_PLUGINS_PATCARDREADER_HPP_

#include "appfwk/cmd/Structs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/app/Nljs.hpp"

#include "lbrulibs/patcardreader/Nljs.hpp"
#include "lbrulibs/patcardreaderinfo/InfoNljs.hpp"

#include "appfwk/DAQModule.hpp"
#include "utilities/WorkerThread.hpp"

#include "HALLinkConcept.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace dunedaq::lbrulibs
{
  class PATCardReader: public dunedaq::appfwk::DAQModule
  {
  public:
    explicit PATCardReader(const std::string &name);
    PATCardReader(const PATCardReader &)              = delete;
    PATCardReader &operator = (const PATCardReader &) = delete;
    PATCardReader(PATCardReader &&)                   = delete;
    PATCardReader &operator = (PATCardReader &&)      = delete;

    void init(const data_t &args) override;
    void get_info(opmonlib::InfoCollector &ci, int level) override;

  private:
    using module_conf_t = dunedaq::lbrulibs::patcardreader::Conf;
    static constexpr size_t m_queue_capacity = 1000000;

    void do_configure(const data_t &args);
    void do_start    (const data_t &args);
    void do_stop     (const data_t &args);

    bool m_configured;
    module_conf_t m_cfg;

    int m_card_id;

    std::map<int, std::unique_ptr<HALLinkConcept>> m_hallink;
  };
}

#endif
