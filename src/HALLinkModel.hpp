#ifndef LBRULIBS_SRC_HALLINKMODEL_HPP_
#define LBRULIBS_SRC_HALLINKMODEL_HPP_

#include "HALLinkConcept.hpp"
#include "uhal/uhal.hpp"

#include "iomanager/IOManager.hpp"
#include "iomanager/Sender.hpp"
#include "logging/Logging.hpp"

#include "readoutlibs/utils/ReusableThread.hpp"
#include "ndreadoutlibs/NDReadoutPATTypeAdapter.hpp"

#include <nlohmann/json.hpp>
#include <folly/ProducerConsumerQueue.h>

#include <string>
#include <mutex>
#include <atomic>
#include <memory>
#include <chrono>

enum
  {
    TLVL_ENTER_EXIT_METHODS = 5,
    TLVL_WORK_STEPS         = 10,
    TLVL_BOOKKEEPING        = 15
  };

namespace dunedaq::lbrulibs
{
  template<class TargetPayloadType>
  class HALLinkModel: public HALLinkConcept
  {
  public:
    using sink_t = iomanager::SenderConcept<TargetPayloadType>;
    using data_t = nlohmann::json;

    HALLinkModel(): HALLinkConcept(), m_run_marker{false}, m_parser_thread(0)
    { }

    ~HALLinkModel() {}

    void set_sink(const std::string &sink_name) override
    {
      if(m_sink_is_set)
	{
	  TLOG(TLVL_WORK_STEPS) << "HALLinkModel sink is already set and initialised!";
	}
      else
	{
	  m_sink_queue  = get_iom_sender<TargetPayloadType>(sink_name);
	  m_sink_is_set = true;
	}    
    }

    void init(const data_t &)
    {
      TLOG_DEBUG(5) << "HALLinkModel init: nothing to do!";
    }

    void conf(const data_t &args)
    {
      if(m_configured)
	{
	  TLOG(TLVL_WORK_STEPS) << "HALLinkModel is already configured!";
	}
      else
	{
	  m_cfg = args.get<patcardreader::Conf>();
	  TLOG(TLVL_WORK_STEPS) << "Configuring HALLinkModel!";
	  m_queue_timeout = std::chrono::milliseconds(m_cfg.mhal_receiver_timeout); //1000miliseconds I think
	  TLOG(TLVL_WORK_STEPS) << "HALLinkModel conf: initialising HAL connection manager!";
	  m_dev_connected = false;
	  TLOG(TLVL_WORK_STEPS) << "HALLinkModel conf: connecting HAL endpoint!";
	  set_names(m_cfg.board_name, m_cfg.dev_name);
	  m_cm  = std::make_unique<uhal::ConnectionManager>(m_board_name);
	  m_dev = std::make_unique<uhal::HwInterface>(m_cm->getDevice(m_dev_name));
	  m_dev_connected = true;
	  TLOG(TLVL_WORK_STEPS) << "HALLinkModel conf: set parser thread name!";
	  m_parser_thread.set_name(m_BOARDLink_sourceLink, m_link_tag);
	  m_configured = true;	  
	}
    }

    void start(const data_t &)
    {
      if (!m_run_marker.load())
	{
	  set_running(true);
	  m_parser_thread.set_work(&HALLinkModel::process_HALLink, this);
	  TLOG_DEBUG(5) << "Started HALLinkModel...";
	}
      else
	{
	  TLOG_DEBUG(5) << "HALLinkModel is already running!";
	}
    }    

    void stop(const data_t&)
    {
      if(m_run_marker.load())
	{
	  set_running(false);
	  while(!m_parser_thread.get_readiness())
	    {
	      std::this_thread::sleep_for(std::chrono::milliseconds(10));
	    }
	  TLOG_DEBUG(5) << "Stopped HALLinkModel!";	  
	}
      else
	{
	  TLOG_DEBUG(5) << "HALLinkModel is already stopped!";
	}
    }

    void set_running(bool should_run)
    {
      bool was_running = m_run_marker.exchange(should_run);
      TLOG_DEBUG(5) << "Active state was toggled from " << was_running << " to " << should_run;
    }

    bool queue_in_message_address(uint64_t addr)
    {
      if(m_message_addr_queue->write(addr)) return true;
      else                                  return false;
    }

    void init(const data_t &, const size_t)
    { }
    
  private:

    using UniqueMessageAddrQueue = std::unique_ptr<folly::ProducerConsumerQueue<uint64_t>>;

    std::atomic<bool> m_run_marker;
    bool m_configured{false};

    bool m_sink_is_set{false};
    std::shared_ptr<sink_t> m_sink_queue;

    UniqueMessageAddrQueue m_message_addr_queue;
    size_t m_packetCounter = 0;
    int m_packetsizesum    = 0;
    int m_packetsize       = 0;
    uint64_t m_timestamp   = 0;
    int m_rcvd_zero        = 0;

    std::vector<std::vector<uint32_t>> m_data;
    std::vector<uint32_t> m_left_over;

    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::high_resolution_clock::now();

    inline static const std::string m_parser_thread_name = "HALLinkp";
    readoutlibs::ReusableThread m_parser_thread;

    virtual void get_info(opmonlib::InfoCollector &ci, int)
    {
      dunedaq::lbrulibs::patcardreaderinfo::HALLinkInfo linkInfo;
      std::chrono::time_point<std::chrono::system_clock> t_end = std::chrono::high_resolution_clock::now();
      double elapsed_time = std::chrono::duration<double>(t_end-t_start).count();
      t_start = t_end;

      //PAT varibles
      linkInfo.bandwidth                   = m_packetsizesum/(elapsed_time*1000000);
      linkInfo.num_packets_received        = m_packetCounter;
      linkInfo.last_packet_size            = m_packetsize;
      linkInfo.last_message_timestamp      = m_timestam;
      linkInfo.subscriber_num_zero_packets = m_rcvd_zero;
      linkInfo.link_tag                    = m_link_tag;
      linkInfo.card_id                     = m_card_id;
      linkInfo.sink_name                   = m_sink_queue->get_name();
      linkInfo.subscriber_connected        = m_dev_connected;
      linkInfo.run_marker                  = m_run_marker;
      linkInfo.sink_is_set                 = m_sink_is_set;
      linkInfo.source_link_string          = m_BOARDLink_sourceLink;
      linkInfo.board_name                  = m_board_name; //Extra
      linkInfo.dev_name                    = m_dev_name; //Extra
      
      m_packetsizesum = 0;
      ci.add(linkInfo);
    }

    void load_temp_buffer(std::vector<uint32_t> buffer)
    {
      std::vector<uint32_t> t_buffer = m_left_over;
      for(int i_word = 0; i_word < (int)buffer.size(); i_word++)
	{
	  TLOG_DEBUG(1) << "Word: " << buffer[i_word];
	  t_buffer.push_back(buffer[i_word]);
	  if(buffer[i_word] == 4294967295)
	    {
	      m_data.push_back(t_buffer);
	      t_buffer.clear();
	    }
	}
      m_left_over.clear();
      for(int i_left = 0; i_left < (int)t_buffer.size(); i_left++) m_left_over.push_back(t_buffer[i_left]);	  
    }
    
    void process_HALLink()
    {
      TLOG_DEBUG(1) << "Starting HAL link process";

      std::ostringstream oss;
      
      while(m_run_marker.load())
	{
	  TLOG_DEBUG(1) << "Looping";
	  if(m_dev_connected)
	    {
	      TLOG_DEBUG(1) << ": Ready to receive data";
	      uhal::ValWord<uint32_t> mon = m_dev->getNode("mon_reg").read();
	      m_dev->dispatch();
	      uint16_t bufSize = (uint16_t)mon.value();
	      if(bufSize == 0)
		{
		  m_rcvd_zero++;
		  TLOG_DEBUG(1) << "No data received, moving to next loop iteration";
		  continue;
		}
	      TLOG_DEBUG(1) << "N = " << bufSize << " data to be read!";
	      uhal::ValVector<uint32_t> msg = m_dev->getNode("fifo_reg").readBlock(bufSize);
	      m_dev->dispatch();
	      try
		{
		  TargetPayloadType *Payload = new TargetPayloadType();
		  load_temp_buffer(msg.value());
		  TLOG_DEBUG(1) << "Data Buffer size: " << (int)m_data[0].size() << " received data: "<< (int)msg.value().size();
		  for(int i_pkt = 0; i_pkt < (int)m_data.size(); i_pkt++)
		    {
		      TLOG_DEBUG(1) << "Pkt: "<< i_pkt;
		      Payload->load_message((void*)&m_data[i_pkt][0],m_data[i_pkt].size());
		      TLOG_DEBUG(1) << "Packet loaded!!";
		      m_timestamp = Payload->get_timestamp();
		      TLOG_DEBUG(1) << "Time stamp = " << m_timestamp;
		      m_sink_queue->send(std::move(*Payload), m_sink_timeout);
		      m_packetsizesum += m_data[i_pkt].size();
		    }
		  m_packetsize     = msg.size();
		}
	      catch(const iomanager::TimeoutExpired &ex)
		{
		  ers::warning(ex);
		}
	      TLOG_DEBUG(1) << ": End of do_word loop";
	      m_packetCounter++;
	    }
	  else
	    {
	      TLOG_DEBUG(1) << "Device not yet connected";
	    }
	}
    }

  };
}


#endif
