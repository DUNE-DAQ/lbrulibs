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

#include "readout/ReusableThread.hpp"
#include "appfwk/DAQSink.hpp"
#include "logging/Logging.hpp"

#include <nlohmann/json.hpp>
#include <folly/ProducerConsumerQueue.h>

#include "ipm/Subscriber.hpp"

#include <string>
#include <mutex>
#include <atomic>
#include <memory>

namespace dunedaq::ipm {

template<class TargetPayloadType>
class ZMQLinkModel : public ZMQLinkConcept {
public:
  using sink_t = appfwk::DAQSink<TargetPayloadType>;
  using inherited = ZMQLinkConcept;
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
    std::shared_ptr<Subscriber> subscriber=makeIPMReceiver("ZmqSubscriber");
    subscriber->connect_for_receives({ {"connection_string", inherited::m_ZMQLink_sourceLink} });
  }

  void conf(const data_t& /*args*/) {
    if (m_configured) {
      TLOG_DEBUG(5) << "ZMQLinkModel is already configured!";
    } else {
      m_parser_thread.set_name(inherited::m_ZMQLink_sourceLink);
      subscriber->subscribe("");
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
    size_t counter = 0;
    std::ostringstream oss;
    while (m_run_marker.load()) {
        if (m_input->can_receive()) {
            TLOG_DEBUG(1) << get_name() << ": Creating output vector";
            std::vector<std::byte> output();
        try {
            auto recvd = m_input->receive(inherited::m_queue_timeout);
            if (recvd.data.size() == 0) {
                TLOG_DEBUG(1) << "No data received, moving to next loop iteration";
                continue;
            }

            memcpy(&output[0], &recvd.data[0]);

            oss << ": Received vector " << counter << " with size " << output.size();
            ers::info(SubscriberProgressUpdate(ERS_HERE, get_name(), oss.str()));
            oss.str("");
        } catch (ReceiveTimeoutExpired const& rte) {
        TLOG_DEBUG(1) << "ReceiveTimeoutExpired: " << rte.what();
        continue;
        }

        TLOG_DEBUG(1) << get_name() << ": Pushing vector into output_queue";
        try {
            m_sink_queue->push(std::move(output), inherited::m_queue_timeout);
        } catch (const appfwk::QueueTimeoutExpired& ex) {
            ers::warning(ex);
        }

        TLOG_DEBUG(1) << get_name() << ": End of do_work loop";
        counter++;
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
  }
};
  
} // namespace dunedaq::ipm

#endif // LBRULIBS_SRC_ZMQLINKMODEL_HPP_
