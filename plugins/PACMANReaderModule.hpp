/**
 * @file PACMANReaderModule.hpp PACMAN card reader DAQ Module.
 *
 * This is part of the DUNE DAQ , copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef LBRULIBS_PLUGINS_PACMANCARDREADER_HPP_
#define LBRULIBS_PLUGINS_PACMANCARDREADER_HPP_

#include "appfwk/cmd/Structs.hpp"
// #include "appfwk/cmd/Nljs.hpp"
// #include "appfwk/app/Nljs.hpp"

// #include "lbrulibs/pacmancardreader/Nljs.hpp"
// #include "lbrulibs/pacmancardreaderinfo/InfoNljs.hpp"

// From appfwk
#include "appfwk/DAQModule.hpp"
#include "utilities/WorkerThread.hpp"

#include "ZMQLinkConcept.hpp"
#include "STREAMLinkConcept.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace dunedaq::lbrulibs {

class PACMANReaderModule : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief PACMANReaderModule Constructor
   * @param name Instance name for this PACMANReaderModule instance
   */
  explicit PACMANReaderModule(const std::string& name);

  PACMANReaderModule(const PACMANReaderModule&) =
    delete; ///< PACMANReaderModule is not copy-constructible
  PACMANReaderModule& operator=(const PACMANReaderModule&) =
    delete; ///< PACMANReaderModule is not copy-assignable
  PACMANReaderModule(PACMANReaderModule&&) =
    delete; ///< PACMANReaderModule is not move-constructible
  PACMANReaderModule& operator=(PACMANReaderModule&&) =
    delete; ///< PACMANReaderModule is not move-assignable

  void init(std::shared_ptr<appfwk::ConfigurationManager> mcfg) override;
  // void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  // Types
  // using module_conf_t = dunedaq::lbrulibs::pacmancardreader::Conf;

  // Constants
  static constexpr size_t m_queue_capacity = 1000000;

  // Commands
  void do_configure(const CommandData_t& args);
  void do_start(const CommandData_t& args);
  void do_stop(const CommandData_t &args);

  // Configuration
  bool m_configured;
  // module_conf_t m_cfg;

  int m_card_id;
  float m_zmq_receiver_timeout;
  std::vector<unsigned int> m_link_confs;

  // Card object
  //std::unique_ptr<PACMANReaderModule> m_card_wrapper;

  // ZMQLinkConcept
  std::map<int, std::unique_ptr<ZMQLinkConcept>> m_zmqlink;
  // STREAMLinkConcept
  std::map<int, std::unique_ptr<STREAMLinkConcept>> m_streamlink;

};

} // namespace dunedaq::lbrulibs

#endif // LBRULIBS_PLUGINS_PACMANCARDREADER_HPP_
