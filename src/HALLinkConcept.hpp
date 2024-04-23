#ifndef LBRULIBS_SRC_HALLINKCONCEPT_HPP_
#define LBRULIBS_SRC_HALLINKCONCEPT_HPP_

#include "uhal/uhal.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <sstream>
#include <string>

namespace dunedaq
{
  namespace lbrulibs
  {
    class HALLinkConcept
    {
    public:
      HALLinkConcept(): m_card_id(0), m_link_tag(0)
      { }

      // HALLinkConcept(std::string board_name, std::string dev_name): m_card_id(0), m_link_tag(0)
      // {
      // 	m_board_name = board_name;	
      // 	m_dev_name   = dev_name;
      // }
      
      virtual ~HALLinkConcept()
      { }
      
      HALLinkConcept(const HALLinkConcept&)            = delete;
      HALLinkConcept &operator=(const HALLinkConcept&) = delete;
      HALLinkConcept(HALLinkConcept &&)                = delete;
      HALLinkConcept &operator=(HALLinkConcept &&)     = delete;

      virtual void init(const nlohmann::json &args, const size_t queue_capacity) = 0;
      virtual void set_sink(const std::string &sink_name)                        = 0;
      virtual void conf(const nlohmann::json &args)                              = 0;
      virtual void start(const nlohmann::json &args)                             = 0;
      virtual void stop(const nlohmann::json &args)                              = 0;
      virtual void get_info(opmonlib::InfoCollector &ci, int level)              = 0;
      
      void set_ids(int card, int tag)
      {
	m_card_id  = card;
	m_link_tag = tag;
      }
      
      void set_names(std::string board_name, std::string dev_name)
      {
	char temp1[100], temp2[100];
	sprintf(temp1,"file:///home/dmatter/toad_slow_control/OneDrive/TIPAGGR_Testing/xml_files/daq_%s.xml",board_name.c_str()); //Maybe a hack?
	m_board_name = temp1;
	sprintf(temp2,"%s%d"  ,dev_name.c_str(),m_card_id+1);
	m_dev_name   = temp2;
      }

    protected:
      dunedaq::lbrulibs::patcardreader::Conf m_cfg;
      std::chrono::milliseconds m_queue_timeout;
      std::chrono::milliseconds m_sink_timeout{10};
      bool m_dev_connected{false};
      std::unique_ptr<uhal::ConnectionManager> m_cm;
      //uhal::HwInterface m_dev{m_cm.getDevice(m_dev_name)};
      std::unique_ptr<uhal::HwInterface> m_dev;
      int m_card_id;
      int m_link_tag;
      std::string m_dev_name   = "aggr1";
      std::string m_board_name = "file://dmatter/toad_slow_control/OneDrive/TIPAGGR_Testing/xml_files/daq_board.xml";
      std::string m_BOARDLink_sourceLink = "tcp://127.0.0.1:5556";

    private:

    };
  }
}

#endif
