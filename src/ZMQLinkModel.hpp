/**
 * @file ZMQLinkModel.hpp FELIX CR's ZMQLink concept wrapper
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef LBRULIBS_SRC_ZMQLINKMODEL_HPP_
#define LBRULIBS_SRC_ZMQLINKMODEL_HPP_

#include "ZMQLinkConcept.hpp"

#include "readout/utils/ReusableThread.hpp"
#include "appfwk/DAQSink.hpp"
#include "logging/Logging.hpp"

#include <nlohmann/json.hpp>
#include <folly/ProducerConsumerQueue.h>

#include "ipm/Subscriber.hpp"
#include "readout/NDReadoutTypes.hpp"

#include <string>
#include <mutex>
#include <atomic>
#include <memory>

namespace dunedaq::lbrulibs {

template<class TargetPayloadType>
class ZMQLinkModel : public ZMQLinkConcept {
public:
  using sink_t = appfwk::DAQSink<TargetPayloadType>;
  using data_t = nlohmann::json;

  /**
   * @brief ZMQLinkModel Constructor
   * @param name Instance name for this ZMQLinkModel instance
   */
  ZMQLinkModel()
    : ZMQLinkConcept()
    , m_run_marker{false}
    , m_parser_thread(0)
  { }
  ~ZMQLinkModel() { }

  void set_sink(const std::string& sink_name) override {
    if (m_sink_is_set) {
      TLOG_DEBUG(5) << "ZMQLinkModel sink is already set and initialized!";
    } else {
      m_sink_queue = std::make_unique<sink_t>(sink_name);
      m_sink_is_set = true;
    }
  }

  std::unique_ptr<sink_t>& get_sink() {
    return m_sink_queue;
  }

  void init(const data_t& /*args*/) {
    TLOG_DEBUG(5) << "ZMQLinkModel init: nothing to do!";

  }

  void conf(const data_t& /*args*/) {
    if (m_configured) {
      TLOG_DEBUG(5) << "ZMQLinkModel is already configured!";
    } else {

      TLOG_DEBUG(5) << "ZMQLinkModel conf: initialising subscriber!";
      m_subscriber = dunedaq::ipm::make_ipm_subscriber("ZmqSubscriber");
      TLOG_DEBUG(5) << "ZMQLinkModel conf: connecting subscriber!";
      m_subscriber->connect_for_receives({ {"connection_string", ZMQLinkConcept::m_ZMQLink_sourceLink} });
      TLOG_DEBUG(5) << "ZMQLinkModel conf: enacting subscription!";
      m_subscriber->subscribe("");
      TLOG_DEBUG(5) << "Configuring ZMQLinkModel!";

      m_parser_thread.set_name(ZMQLinkConcept::m_ZMQLink_sourceLink, ZMQLinkConcept::m_link_tag);
      m_configured=true;
    } 
  }

  void start(const data_t& /*args*/) {
    if (!m_run_marker.load()) {
      set_running(true);
      m_parser_thread.set_work(&ZMQLinkModel::process_ZMQLink, this);
      TLOG_DEBUG(5) << "Started ZMQLinkModel...";
    } else {
      TLOG_DEBUG(5) << "ZMQLinkModel is already running!";
    }
  }

  void stop(const data_t& /*args*/) {
    if (m_run_marker.load()) {
      set_running(false);
      while (!m_parser_thread.get_readiness()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      TLOG_DEBUG(5) << "Stopped ZMQLinkModel!";
    } else {
      TLOG_DEBUG(5) << "ZMQLinkModel is already stopped!";
    }
  }

  void set_running(bool should_run) {
    bool was_running = m_run_marker.exchange(should_run);
    TLOG_DEBUG(5) << "Active state was toggled from " << was_running << " to " << should_run;
  }


  bool queue_in_message_address(uint64_t addr) { // NOLINT
    if (m_message_addr_queue->write(addr)) { // ok write
      return true;
    } else { // failed write
      return false;
    }
  } 

   void init(const data_t& /*args*/, const size_t /*block_queue_capacity*/)
  {
    //Required by parent class
  }


private:
  // Types
  using UniqueMessageAddrQueue = std::unique_ptr<folly::ProducerConsumerQueue<uint64_t>>; // NOLINT

  // Internals
  std::atomic<bool> m_run_marker;
  bool m_configured{false};

  // Sink
  bool m_sink_is_set{false};
  std::unique_ptr<sink_t> m_sink_queue;

  // mesages to process
  UniqueMessageAddrQueue m_message_addr_queue;

  // Processor
  inline static const std::string m_parser_thread_name = "ZMQLinkp";
  readout::ReusableThread m_parser_thread;
  void process_ZMQLink() {

    TLOG_DEBUG(1) << "Starting ZMQ link process";

    size_t counter = 0;
    std::ostringstream oss;
    while (m_run_marker.load()) {
        TLOG_DEBUG(1) << "Looping";
        
        if (m_subscriber->can_receive()) {
            TLOG_DEBUG(1) << ": Ready to receive data";
        try {
            auto recvd = m_subscriber->receive(ZMQLinkConcept::m_queue_timeout);
            if (recvd.data.size() == 0) {
                TLOG_DEBUG(1) << "No data received, moving to next loop iteration";
                continue;
            }
            TLOG_DEBUG(1) << ": Pushing data into output_queue";
            try {
              TargetPayloadType* Payload = new TargetPayloadType();
              std::memcpy((void *)&Payload->data, &recvd.data[0], recvd.data.size());

              m_sink_queue->push(*Payload, ZMQLinkConcept::m_queue_timeout);
            } catch (const appfwk::QueueTimeoutExpired& ex) {
              ers::warning(ex);
            }

        } catch (dunedaq::ipm::ReceiveTimeoutExpired const& rte) {
        TLOG_DEBUG(1) << "ReceiveTimeoutExpired: " << rte.what();
        continue;
        }
        TLOG_DEBUG(1) << ": End of do_work loop";
        counter++;
        } else {
            TLOG_DEBUG(1) << "Sleeping";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
  }
};
  
} // namespace dunedaq::lbrulibs

#endif // LBRULIBS_SRC_ZMQLINKMODEL_HPP_
