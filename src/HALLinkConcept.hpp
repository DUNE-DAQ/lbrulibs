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

      HALLinkConcept(std::string board_name, std::string dev_name): m_card_id(0), m_link_tag(0)
      {
	m_board_name = board_name;	
	m_dev_name   = dev_name;
      }

      virtual ~HALLinkConcept()
      { }
      
      HALLinkConcept(const HALLinkConcept&)            = delete;
      HALLinkConcept &operator=(const HALLinkConcept&) = delete;
      HALLinkConcept(HALLinkConcept &&)                = delete;
      HALLinkConcept &operator=(HALLinkConcept &&)     = delete;

      virtual void init(const nlohmann::json &args, const size_t queue_capacity) = 0;
      virtual void set_sink(const std::string &sink_name)                        = 0;
      virtual void conf(const nlohmann::json &args)                               = 0;
      virtual void start(const nlohmann::json &args)                              = 0;
      virtual void stop(const nlohmann::json &args)                               = 0;
      virtual void get_info(opmonlib::InfoCollector &ci, int level)              = 0;
      
      void set_ids(int card, int tag)
      {
	m_card_id  = card;
	m_link_tag = tag;
      }
      
      void set_names(std::string board_name, std::string dev_name)
      {
	char temp1[100], temp2[100];
	sprintf(temp1,"$XML_PATH/%s.xml",board_name.c_str()); //Maybe a hack?
	m_board_name = temp1;
	sprintf(temp2,"%s%d"  ,dev_name.c_str(),m_card_id);
	m_dev_name   = temp2;
      }

    protected:
      dunedaq::lbrulibs::patcardreader::Conf m_cfg;
      std::chrono::milliseconds m_queue_timeout;
      std::chrono::milliseconds m_sink_timeout{10};
      bool m_dev_connected{false};
      uhal::ConnectionManager m_cm{m_board_name};
      uhal::HwInterface m_dev{m_cm.getDevice(m_dev_name)};
      int m_card_id;
      int m_link_tag;
      std::string m_dev_name;
      std::string m_board_name;
      std::string m_BOARD_sourceLink = "pat";

    private:

    };
  }
}

#endif
