/**
 * @file PACMANReaderModule.cc PACMANReaderModule DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "PACMANReaderModule.hpp"
#include "CreateSTREAMLink.hpp"
#include "CreateZMQLink.hpp"
#include "ZMQIssues.hpp"
#include "logging/Logging.hpp"

// TODO: Remove unecessary includes
#include "appmodel/PACMANReceiver.hpp"
#include "appmodel/PACMANConfiguration.hpp"
#include "confmodel/ResourceSetAND.hpp"
#include "confmodel/Connection.hpp"
#include "confmodel/QueueWithSourceId.hpp"
#include "confmodel/DetectorStream.hpp"
#include "confmodel/DetectorToDaqConnection.hpp"
#include "confmodel/GeoId.hpp"
#include "appmodel/DataReaderModule.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

bool usePUBSUB = 1;

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "PACMANReaderModule" // NOLINT

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

    PACMANReaderModule::PACMANReaderModule(const std::string& name)
      : DAQModule(name)
      , m_configured(false)
      , m_card_id(0)

    {
      register_command("conf", &PACMANReaderModule::do_configure);
      register_command("start", &PACMANReaderModule::do_start);
      register_command("stop", &PACMANReaderModule::do_stop);
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
    PACMANReaderModule::init(std::shared_ptr<appfwk::ModuleConfiguration> mcfg){
      TLOG() << "Running init on PACMAN cards";
      auto modconf = mcfg->module<appmodel::DataReaderModule>(get_name());
      if (modconf->get_connections().size() != 1){
        throw InitializationError(ERS_HERE, "PACMAN Data Reader does not have a unique associated interface");
      }

      const confmodel::DetectorToDaqConnection*  det_con = modconf->get_connections()[0]->cast<confmodel::DetectorToDaqConnection>();

      // Create a source_id to local elink map

      for (const auto & resources : det_con->get_contains()) {
        const appmodel::PACMANReceiver* interface = resources->cast<appmodel::PACMANReceiver>();

        if (interface != nullptr){
          //m_card_wrapper = std::make_unique<PACMANReaderModule>(interface);
          auto module_conf = interface->get_configuration()->cast<appmodel::PACMANConfiguration>();
          // m_card_id = module_conf->get_card();
          m_zmq_receiver_timeout = module_conf->get_zmq_receiver_timeout();
          // m_link_confs = module_conf->get_link_confs();
        }
      }



      for(auto qi : modconf->get_outputs()){
        auto q_with_id = qi->cast<confmodel::QueueWithSourceId>();
        if (q_with_id == nullptr) {
          ers::fatal(InitializationError(ERS_HERE, "No Queues Found"));
          continue;
        }

        if(usePUBSUB){
          TLOG() << "Creating ZMQLinkModel for target queue: " << q_with_id->UID() << " DLH number: " << q_with_id->get_source_id();
          // TODO : Resolve proper link ID here
          m_zmqlink[0] = createZMQLinkModel(q_with_id->UID());
          if(m_zmqlink[0]==nullptr){
            ers::fatal(InitializationError(ERS_HERE, "CreateZMQLink failed to provide an appropriate model for queue!"));
          }
          m_zmqlink[0]->init(m_queue_capacity);

        } else{
          TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating STREAMLinkModel for target queue: " << q_with_id->UID() << " DLH number: " << q_with_id->get_source_id();

          if(m_streamlink[0]==nullptr){
            ers::fatal(InitializationError(ERS_HERE, "CreateSTREAMLink failed to provide an appropriate model for queue!"));
          }
          m_streamlink[0]->init(m_queue_capacity);
        }
      }
      if(m_streamlink.empty() && m_zmqlink.empty()){
        ers::fatal(InitializationError(ERS_HERE, "No Cards have been initialised, this will just break"));
      }

    }

    void PACMANReaderModule::do_configure(const data_t& /*args*/){
      // Configure Components
      TLOG() << get_name() << ": Entering do_conf() method";

      if (usePUBSUB) {
        TLOG(TLVL_WORK_STEPS) << "Using ZMQ Publish/Subscribe";

        if(!m_zmqlink[0]){
          ers::fatal(InitializationError(ERS_HERE, "No ZMQ Card Exists!!"));
        }

        m_zmqlink[0]->set_ids(m_card_id, 0);
        m_zmqlink[0]->conf(m_zmq_receiver_timeout);
      } else {
        TLOG(TLVL_WORK_STEPS) << "Using Raw TCP Stream";
        m_streamlink[0]->set_ids(m_card_id, 0);
        TLOG(TLVL_WORK_STEPS) << "apply conf";
        m_streamlink[0]->conf(m_zmq_receiver_timeout);
        TLOG(TLVL_WORK_STEPS) << "finish conf";
      }
    }

    void
    PACMANReaderModule::do_start(const data_t& /*args*/)
    {
      if (usePUBSUB) {
        m_zmqlink[0]->start();
      } else {
        m_streamlink[0]->start();
      }
    }

    void
    PACMANReaderModule::do_stop(const data_t& /*args*/)
    {
      if (usePUBSUB) {
        m_zmqlink[0]->stop();
      } else {
        m_streamlink[0]->stop();
      }
    }

    // void
    // PACMANReaderModule::get_info(opmonlib::InfoCollector& ci, int level)
    // {
    //   if (usePUBSUB) {
    //     m_zmqlink[0]->get_info(ci, level);
    //   } else {
    //     m_streamlink[0]->get_info(ci, level);
    //   }
    // }

  } // namespace lbrulibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::lbrulibs::PACMANReaderModule)
