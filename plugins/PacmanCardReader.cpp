/**
 * @file PacmanCardReader.cc PacmanCardReader DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "PacmanCardReader.hpp"
#include "CreateZMQLink.hpp"

#include "logging/Logging.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <memory>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "PacmanCardReader" // NOLINT

/**
 * @brief TRACE debug levels used in this source file
 */
enum
{
  TLVL_ENTER_EXIT_METHODS = 5,
  TLVL_WORK_STEPS = 10,
  TLVL_BOOKKEEPING = 15
};

namespace dunedaq {
namespace lbrulibs {

PacmanCardReader::PacmanCardReader(const std::string& name)
  : DAQModule(name)
  , m_configured(false)
  , m_card_id(0)
  , m_logical_unit(0)
  , m_router(nullptr)

{
  register_command("conf", &PacmanCardReader::do_configure);
  register_command("start", &PacmanCardReader::do_start);
  register_command("stop", &PacmanCardReader::do_stop);
}

inline void
tokenize(std::string const &str, const char delim,
         std::vector<std::string>& out)
{
  std::size_t start;
  std::size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
}

void
PacmanCardReader::init(const data_t& args)
{
  auto ini = args.get<appfwk::app::ModInit>();
  for (const auto& qi : ini.qinfos) {
    if (qi.dir != "output") {
      ers::error(InitializationError(ERS_HERE, "Only output queues are supported in this module!"));
      continue;
    } else {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "PacmanCardReader output queue is " << qi.inst;
      const char delim = '_';
      std::string target = qi.inst;
      std::vector<std::string> words;
      tokenize(target, delim, words);
#warning RS FIXME -> Unhandled potential exception.
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating ElinkModel for target queue: " << target; 
      m_zmqlink = createZMQLinkModel(qi.inst);
      m_zmqlink->init(args, m_queue_capacity);
    }
  }
}

void
PacmanCardReader::do_configure(const data_t& args)
{
  m_cfg = args.get<pacmancardreader::Conf>();
  m_card_id = m_cfg.card_id;
  m_logical_unit = m_cfg.logical_unit;
  
  // Config checks - make some if config values needed, felix example below
  /*
  if (m_num_links != m_elinks.size()) {
    ers::fatal(ElinkConfigurationInconsistency(ERS_HERE, m_num_links));
  } 
  */

  // Configure components
  TLOG(TLVL_WORK_STEPS) << "Configuring ZMQLinkHandler";
  m_zmqlink->set_id(m_card_id, m_logical_unit, lid, tag);
  m_zmqlink->conf(args);
}

void
PacmanCardReader::do_start(const data_t& args)
{
  m_zmqlink->start(args);
}

void
PacmanCardReader::do_stop(const data_t& args)
{
  m_zmqlink->stop(args);
}

} // namespace lbrulibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::lbrulibs::PacmanCardReader)
