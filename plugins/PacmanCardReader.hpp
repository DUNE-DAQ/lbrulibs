/**
 * @file PacmanCardReader.hpp PACMAN card reader DAQ Module.
 *
 * This is part of the DUNE DAQ , copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef LBRULIBS_PLUGINS_PACMANCARDREADER_HPP_
#define LBRULIBS_PLUGINS_PACMANCARDREADER_HPP_

#include "appfwk/cmd/Structs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/app/Nljs.hpp"

#include "lbrulibs/pacmancardreader/Nljs.hpp"
#include "lbrulibs/pacmancardreaderinfo/InfoNljs.hpp"

// From appfwk
#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSink.hpp"
#include "utilities/WorkerThread.hpp"

#include "ZMQLinkConcept.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace dunedaq::lbrulibs {

class PacmanCardReader : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief PacmanCardReader Constructor
   * @param name Instance name for this PacmanCardReader instance
   */
  explicit PacmanCardReader(const std::string& name);

  PacmanCardReader(const PacmanCardReader&) =
    delete; ///< PacmanCardReader is not copy-constructible
  PacmanCardReader& operator=(const PacmanCardReader&) =
    delete; ///< PacmanCardReader is not copy-assignable
  PacmanCardReader(PacmanCardReader&&) =
    delete; ///< PacmanCardReader is not move-constructible
  PacmanCardReader& operator=(PacmanCardReader&&) =
    delete; ///< PacmanCardReader is not move-assignable

  void init(const data_t& args) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  // Types
  using module_conf_t = dunedaq::lbrulibs::pacmancardreader::Conf;
  
  // Constants
  static constexpr size_t m_queue_capacity = 1000000;

  // Commands
  void do_configure(const data_t& args);
  void do_start(const data_t& args);
  void do_stop(const data_t& args);

  // Configuration
  bool m_configured;
  module_conf_t m_cfg;

  int m_card_id;
  
  // ZMQLinkConcept
  std::map<int, std::unique_ptr<ZMQLinkConcept>> m_zmqlink;

};

} // namespace dunedaq::lbrulibs

#endif // LBRULIBS_PLUGINS_PACMANCARDREADER_HPP_
