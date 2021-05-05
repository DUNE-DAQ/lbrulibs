/**
* @file ZMQLinkConcept.hpp ZMQLinkConcept for constructors and
* forwarding command args. Enforces the implementation to 
* queue in block_addresses
*
* This is part of the DUNE DAQ , copyright 2021.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef LBRULIBS_SRC_ZMQLINKCONCEPT_HPP_
#define LBRULIBS_SRC_ZMQLINKCONCEPT_HPP_

#include "DefaultParserImpl.hpp"

#include <nlohmann/json.hpp>
#include "ipm/Subscriber.hpp"

#include <memory>
#include <sstream>
#include <string>

namespace dunedaq {
namespace lbrulibs {

class ZMQLinkConcept {
public:
  ZMQLinkConcept()
    : m_card_id(0)
    , m_logical_unit(0)
  {}
  ~ZMQLinkConcept() {}

  ZMQLinkConcept(const ZMQLinkConcept&)
    = delete; ///< ZMQLinkConcept is not copy-constructible
  ZMQLinkConcept& operator=(const ZMQLinkConcept&)
    = delete; ///< ZMQLinkConcept is not copy-assginable
  ZMQLinkConcept(ZMQLinkConcept&&)
    = delete; ///< ZMQLinkConcept is not move-constructible
  ZMQLinkConcept& operator=(ZMQLinkConcept&&)
    = delete; ///< ZMQLinkConcept is not move-assignable

  virtual void init(const nlohmann::json& args, const size_t queue_capacity) = 0;
  virtual void set_sink(const std::string& sink_name) = 0;
  virtual void conf(const nlohmann::json& args) = 0; //add configuration variables later if needed
  virtual void start(const nlohmann::json& args) = 0;
  virtual void stop(const nlohmann::json& args) = 0;


    void set_ids(int card, int slr) {
        m_card_id = card;
        m_logical_unit = slr;

        std::ostringstream lidstrs;
        lidstrs << "ZMQLink["
                << "cid:" << std::to_string(m_card_id) << "|"
                << "slr:" << std::to_string(m_logical_unit) << "|";
        m_ZMQLink_commandLink = lidstrs.str();

        std::ostringstream tidstrs;
        tidstrs << "ept-" << std::to_string(m_card_id) 
                << "-" << std::to_string(m_logical_unit);
        m_ZMQLink_sourceLink = tidstrs.str();
    }

protected:
    std::shared_ptr<Subscriber> m_input;
    std::chrono::milliseconds m_queue_timeout;
    int m_card_id;
    int m_logical_unit;
    std::string m_ZMQLink_commandLink = "tcp://127.0.0.1:5555";
    std::string m_ZMQLink_sourceLink = "tcp://127.0.0.1:5556";
private:

};

} // namespace lbrulibs
} // namespace dunedaq

#endif // LBRULIBS_SRC_ZMQLINKCONCEPT_HPP_
