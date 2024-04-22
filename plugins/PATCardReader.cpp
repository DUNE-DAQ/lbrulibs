#include "PATCardReader.hpp"
#include "CreateHALLink.hpp"
#include "logging/Logging.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#define TRACE_NAME "PATCardReader"

namespace dunedaq
{
  namespace lbrulibs
  {
    PATCardReader::PATCardReader(const std::string &name): DAQModule(name), m_configured(false), m_card_id(0)
    {
      register_command("conf" , &PATCardReader::do_configure);
      register_command("start", &PATCardReader::do_start    );
      register_command("stop" , &PATCardReader::do_start    );
    }

    inline void tokenize(std::string const &str, const char delim, std::vector<std::string> &out)
    {
      std::size_t start;
      std::size_t end   = 0;
      while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
	  end = str.find(delim, start);
	  out.push_back(str.substr(start, end - start));
	}
    }

    void PATCardReader::init(const data_t &args)
    {
      auto ini = args.get<appfwk::app::ModInit>();
      TLOG(TLVL_WORK_STEPS) << "ini";
      for (const auto &cr: ini.conn_refs)
	{
	  TLOG(TLVL_WORK_STEPS) << "PATCardReader output queue is " << cr.uid;
	  const char delim = '_';
	  std::string target = cr.uid;
	  std::vector<std::string> words;
	  tokenize(target, delim, words);
	  TLOG(TLVL_WORK_STEPS) << "Create HALLinkModel for target queue: " << target;
	  m_hallink[0] = createHALLinkModel(cr.uid);
	  if (m_hallink[0] == nullptr)
	    {
	      ers::fatal(InitializationError(ERS_HERE, "CreateHALLink failed to provide an appropriate model for queue!"));
	    }
	  m_hallink[0]->init(args, m_queue_capacity);
	}
      m_cfg = args.get<patcardreader::Conf>();
    }

    void PATCardReader::do_configure(const data_t &args)
    {
      m_cfg     = args.get<patcardreader::Conf>();
      m_card_id = m_cfg.card_id;
      TLOG(TLVL_WORK_STEPS) << "Configuring LinkHandler";
      m_hallink[0]->set_ids  (m_card_id       , 0             );
      m_hallink[0]->set_names(m_cfg.board_name, m_cfg.dev_name);
      TLOG(TLVL_WORK_STEPS) << "apply conf";
      m_hallink[0]->conf(args);
      TLOG(TLVL_WORK_STEPS) << "finish conf";
    }

    void PATCardReader::do_start(const data_t &args)
    {
      m_hallink[0]->start(args);
    }

    void PATCardReader::do_stop(const data_t &args)
    {
      m_hallink[0]->stop(args);
    }

    void PATCardReader::get_info(opmonlib::InfoCollector &ci, int level)
    {
      m_hallink[0]->get_info(ci, level);
    }
  }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::lbrulibs::PATCardReader)
