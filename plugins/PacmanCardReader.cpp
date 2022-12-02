/**
 * @file PacmanCardReader.cc PacmanCardReader DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "PacmanCardReader.hpp"
#include "CreateSTREAMLink.hpp"
#include "CreateZMQLink.hpp"
#include "ZMQIssues.hpp"
#include "logging/Logging.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

bool usePUBSUB = 0;

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "PacmanCardReader" // NOLINT

/**
 * @brief TRACE debug levels used in this source file
 */
/*enum
{
  TLVL_ENTER_EXIT_METHODS = 5,
  TLVL_WORK_STEPS = 10,
  TLVL_BOOKKEEPING = 15
};
*/
namespace dunedaq {
namespace lbrulibs {

PacmanCardReader::PacmanCardReader(const std::string& name)
  : DAQModule(name)
  , m_configured(false)
  , m_card_id(0)

{
  register_command("conf", &PacmanCardReader::do_configure);
  register_command("start", &PacmanCardReader::do_start);
  register_command("stop", &PacmanCardReader::do_stop);
}

inline void
tokenize(std::string const& str, const char delim, std::vector<std::string>& out)
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
  TLOG(TLVL_WORK_STEPS) << "ini";
  for (const auto& cr : ini.conn_refs) {

    TLOG(TLVL_WORK_STEPS) << "PacmanCardReader output queue is " << cr.uid;
    const char delim = '_';
    std::string target = cr.uid;
    std::vector<std::string> words;
    tokenize(target, delim, words);
    if (usePUBSUB) {
      TLOG(TLVL_WORK_STEPS) << "Creating ZMQLinkModel for target queue: " << target;
      m_zmqlink[0] =
        createZMQLinkModel(cr.uid); // FIX ME - need to resolve proper link ID rather than hard code to zero
      if (m_zmqlink[0] == nullptr) {
        ers::fatal(InitializationError(ERS_HERE, "CreateZMQLink failed to provide an appropriate model for queue!"));
      }
      m_zmqlink[0]->init(args, m_queue_capacity);
    } else {
      TLOG(TLVL_WORK_STEPS) << "Creating STREAMLinkModel for target queue: " << target;
      m_streamlink[0] =
        createSTREAMLinkModel(cr.uid); // FIX ME - need to resolve proper link ID rather than hard code to zero
      if (m_streamlink[0] == nullptr) {
        ers::fatal(InitializationError(ERS_HERE, "CreateSTREAMLink failed to provide an appropriate model for queue!"));
      }
      m_streamlink[0]->init(args, m_queue_capacity);
    }
  }

  m_cfg = args.get<pacmancardreader::Conf>();
}

void
PacmanCardReader::do_configure(const data_t& args)
{
  m_cfg = args.get<pacmancardreader::Conf>();
  m_card_id = m_cfg.card_id;

  // Config checks - make some if config values needed, felix example below
  //
  // if (m_num_links != m_elinks.size()) {
  //  ers::fatal(ElinkConfigurationInconsistency(ERS_HERE, m_num_links));
  //}
  //

  // Configure components
  TLOG(TLVL_WORK_STEPS) << "Configuring LinkHandler";
  if (usePUBSUB) {
    TLOG(TLVL_WORK_STEPS) << "Using ZMQ Publish/Subscribe";
    m_zmqlink[0]->set_ids(m_card_id, 0);
    m_zmqlink[0]->conf(args);
  } else {
    TLOG(TLVL_WORK_STEPS) << "Using Raw TCP Stream";
    m_streamlink[0]->set_ids(m_card_id, 0);
    TLOG(TLVL_WORK_STEPS) << "apply conf";
    m_streamlink[0]->conf(args);
    TLOG(TLVL_WORK_STEPS) << "finish conf";
  }
}

void
PacmanCardReader::do_start(const data_t& args)
{
  if (usePUBSUB) {
    m_zmqlink[0]->start(args);
  } else {
    m_streamlink[0]->start(args);
  }
}

void
PacmanCardReader::do_stop(const data_t& args)
{
  if (usePUBSUB) {
    m_zmqlink[0]->stop(args);
  } else {
    m_streamlink[0]->stop(args);
  }
}

void
PacmanCardReader::get_info(opmonlib::InfoCollector& ci, int level)
{
  if (usePUBSUB) {
    m_zmqlink[0]->get_info(ci, level);
  } else {
    m_streamlink[0]->get_info(ci, level);
  }
}

} // namespace lbrulibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::lbrulibs::PacmanCardReader)
