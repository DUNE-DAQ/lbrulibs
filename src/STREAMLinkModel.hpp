/**
 * @file STREAMLinkModel.hpp STREAMLink concept wrapper
 *
 * This is part of the DUNE DAQ , copyright 2022.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef LBRULIBS_SRC_STREAMLINKMODEL_HPP_
#define LBRULIBS_SRC_STREAMLINKMODEL_HPP_

#include "STREAMLinkConcept.hpp"
#include "zmq.hpp"

#include "iomanager/IOManager.hpp"
#include "iomanager/Sender.hpp"
#include "logging/Logging.hpp"

#include "readoutlibs/utils/ReusableThread.hpp"
#include "ndreadoutlibs/NDReadoutTypes.hpp"

#include <nlohmann/json.hpp>
#include <folly/ProducerConsumerQueue.h>

#include <string>
#include <mutex>
#include <atomic>
#include <memory>
#include <chrono>


/**
 * @brief TRACE debug levels used in this source file
 */
enum
  {
    TLVL_ENTER_EXIT_METHODS = 5,
    TLVL_WORK_STEPS = 10,
  TLVL_BOOKKEEPING = 15
  };


namespace dunedaq::lbrulibs {

template<class TargetPayloadType>
class STREAMLinkModel : public STREAMLinkConcept {
public:
  using sink_t = iomanager::SenderConcept<TargetPayloadType>;
  using data_t = nlohmann::json;

  /**
   * @brief STREAMLinkModel Constructor
   * @param name Instance name for this ZMQLinkModel instance
   */
  STREAMLinkModel()
    : STREAMLinkConcept()
    , m_run_marker{false}
    , m_parser_thread(0)
  { }
  ~STREAMLinkModel() { }

  void set_sink(const std::string& sink_name) override {
    if (m_sink_is_set) {
      TLOG(TLVL_WORK_STEPS) << "STREAMLinkModel sink is already set and initialized!";
    } else {
      m_sink_queue = get_iom_sender<TargetPayloadType>(sink_name);
      m_sink_is_set = true;
    }
  }

  //std::shared_ptr<sink_t> get_sink() {
  //  return m_sink_queue;
  //}

  void init(const data_t& /*args*/) {
    TLOG_DEBUG(5) << "STREAMLinkModel init: nothing to do!";
  }

  void conf(const data_t& args) {
    if (m_configured) {
      TLOG(TLVL_WORK_STEPS) << "STREAMLinkModel is already configured!";
    } else {

      m_cfg = args.get<pacmancardreader::Conf>();
      TLOG(TLVL_WORK_STEPS) << "Configuring STREAMLinkModel!";
      m_queue_timeout = std::chrono::milliseconds(m_cfg.zmq_receiver_timeout);
      TLOG(TLVL_WORK_STEPS) << "STREAMLinkModel conf: initialising subscriber!";
      m_subscriber_connected = false;
      //m_subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
      TLOG(TLVL_WORK_STEPS) << "STREAMLinkModel conf: connecting subscriber!";
      m_subscriber.bind(m_STREAMLink_sourceLink);
      m_subscriber_connected = true;
      TLOG(TLVL_WORK_STEPS) << "STREAMLinkModel conf: set parser thread name!";
      //m_subscriber.setsockopt(ZMQ_SUBSCRIBE, "");
     
      m_parser_thread.set_name(m_STREAMLink_sourceLink, m_link_tag);
      m_configured=true;
    } 
  }

  void start(const data_t& /*args*/) {
    if (!m_run_marker.load()) {
      set_running(true);
      m_parser_thread.set_work(&STREAMLinkModel::process_STREAMLink, this);
      TLOG_DEBUG(5) << "Started STREAMLinkModel...";
    } else {
      TLOG_DEBUG(5) << "STREAMLinkModel is already running!";
    }
  }

  void stop(const data_t& /*args*/) {
    if (m_run_marker.load()) {
      set_running(false);
      while (!m_parser_thread.get_readiness()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      TLOG_DEBUG(5) << "Stopped STREAMLinkModel!";
    } else {
      TLOG_DEBUG(5) << "STREAMLinkModel is already stopped!";
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
  std::shared_ptr<sink_t> m_sink_queue;

  // mesages to process
  UniqueMessageAddrQueue m_message_addr_queue;
  size_t m_packetCounter = 0; //number of packets
  int m_packetsizesum = 0; //sum of data across monitoring period
  int m_packetsize = 0; //last packet size
  int m_timestamp = 0; //Header time stamp of last packet
  int m_rcvd_zero = 0; //subscriber times it outputs 0

  std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::high_resolution_clock::now(); //Initial time when monitoring period starts

  // Processor
  inline static const std::string m_parser_thread_name = "ZMQLinkp";
  readoutlibs::ReusableThread m_parser_thread;

  virtual void get_info(opmonlib::InfoCollector& ci, int /*level*/){
    dunedaq::lbrulibs::pacmancardreaderinfo::ZMQLinkInfo linkInfo;

    std::chrono::time_point<std::chrono::system_clock> t_end = std::chrono::high_resolution_clock::now(); //End time when monitoring period ends
    double elapsed_time = std::chrono::duration<double>(t_end-t_start).count(); //Monitoring period quantified
    t_start = t_end; //restarts system clock for next monitoring period

    //Pacman variables -----------
    linkInfo.bandwidth = m_packetsizesum/(elapsed_time*1000000);
    linkInfo.num_packets_received = m_packetCounter;
    linkInfo.last_packet_size = m_packetsize;
    linkInfo.last_message_timestamp = m_timestamp; 
    linkInfo.subscriber_num_zero_packets = m_rcvd_zero;
    linkInfo.link_tag = m_link_tag; //ZMQLinkConcept Variable
    linkInfo.card_id = m_card_id; //ZMQLinkConcept Variable
    linkInfo.sink_name = m_sink_queue->get_name(); //sink queue name
    linkInfo.subscriber_connected = m_subscriber_connected;
    linkInfo.run_marker = m_run_marker; //predefined
    linkInfo.sink_is_set = m_sink_is_set; //If sink succeeded - predefined
    linkInfo.source_link_string = m_STREAMLink_sourceLink; //string variable from ZMQLinkConcept
    //linkInfo.info_type = "ZMQ Link Info";

    m_packetsizesum = 0; //resets the variable, so the sum starts from 0 again

    ci.add(linkInfo);
  }
  
  void process_STREAMLink() {

    TLOG_DEBUG(1) << "Starting ZMQ link process";

    
    std::ostringstream oss;

    zmq::pollitem_t items[] = {{static_cast<void*>(m_subscriber),0,ZMQ_POLLIN,0}};
    while (m_run_marker.load()) {
        TLOG_DEBUG(1) << "Looping";
        
        if (m_subscriber_connected) {
            TLOG_DEBUG(1) << ": Ready to receive data";
            zmq::message_t id; //routing frame
            zmq::message_t msg;
            zmq::poll (&items [0],1,m_queue_timeout);
	    if (items[0].revents & ZMQ_POLLIN){
              m_subscriber.recv(&id); //routing frame
              auto recvd = m_subscriber.recv(&msg);
              if (recvd == 0) {
		m_rcvd_zero++;
                TLOG_DEBUG(1) << "No data received, moving to next loop iteration";
                continue;
              }
              if (msg.data() == 0){//empty message for establishing connections
                TLOG_DEBUG(1) << "No data received, moving to next loop iteration";
                continue;
              }
              TLOG_DEBUG(1) << ": Pushing data into output_queue";
              try {
                TargetPayloadType* Payload = new TargetPayloadType();
		std::memcpy(static_cast<void *>(&Payload->data), msg.data(), msg.size());
		m_timestamp = Payload->get_timestamp();
                m_sink_queue->send(std::move(*Payload), m_sink_timeout);
		m_packetsizesum += msg.size(); //sum of data from packets
	       	m_packetsize = msg.size(); //last packet size
              } catch (const iomanager::TimeoutExpired& ex) {
                ers::warning(ex);
              }

              TLOG_DEBUG(1) << ": End of do_work loop";
              m_packetCounter++;
            }

        } else {
            TLOG_DEBUG(1) << "Subscriber not yet connected";
        }
    }
  }
};
  
} // namespace dunedaq::lbrulibs

#endif // LBRULIBS_SRC_STREAMLINKMODEL_HPP_
