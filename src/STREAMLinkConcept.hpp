/**
 * * @file STREAMLinkConcept.hpp STREAMLinkConcept for constructors and
 * * forwarding command args. Enforces the implementation to 
 * * queue in block_addresses
 * *
 * * This is part of the DUNE DAQ , copyright 202.
 * * Licensing/copyright details are in the COPYING file that you should have
 * * received with this code.
 * */
#ifndef LBRULIBS_SRC_STREAMLINKCONCEPT_HPP_
#define LBRULIBS_SRC_STREAMLINKCONCEPT_HPP_

#include "zmq.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <sstream>
#include <string>

namespace dunedaq {
namespace lbrulibs {

class STREAMLinkConcept {
public:
  STREAMLinkConcept()
    : m_card_id(0)
    , m_link_tag(0)
  {}
  ~STREAMLinkConcept() {}

  STREAMLinkConcept(const STREAMLinkConcept&)
    = delete; ///< STREAMLinkConcept is not copy-constructible
  STREAMLinkConcept& operator=(const STREAMLinkConcept&)
    = delete; ///< STREAMLinkConcept is not copy-assginable
  STREAMLinkConcept(STREAMLinkConcept&&)
    = delete; ///< STREAMLinkConcept is not move-constructible
  STREAMLinkConcept& operator=(STREAMLinkConcept&&)
    = delete; ///< STREAMLinkConcept is not move-assignable

  virtual void init(const nlohmann::json& args, const size_t queue_capacity) = 0;
  virtual void set_sink(const std::string& sink_name) = 0;
  virtual void conf(const nlohmann::json& args) = 0; //add configuration variables later if needed
  virtual void start(const nlohmann::json& args) = 0;
  virtual void stop(const nlohmann::json& args) = 0;
  virtual void get_info(opmonlib::InfoCollector& ci, int level) = 0;

    void set_ids(int card, int tag) {
        m_card_id = card;
        m_link_tag = tag;
    }

protected:
    dunedaq::lbrulibs::pacmancardreader::Conf m_cfg;
    std::chrono::milliseconds m_queue_timeout;
    std::chrono::milliseconds m_sink_timeout{10};
    bool m_subscriber_connected{false};
    zmq::context_t  m_context;
    zmq::socket_t m_subscriber{m_context, zmq::socket_type::stream};
    int m_card_id;
    int m_link_tag;
    std::string m_STREAMLink_sourceLink = "tcp://127.0.0.1:5556";
private:

};

} // namespace lbrulibs
} // namespace dunedaq

#endif // LBRULIBS_SRC_STREAMLINKCONCEPT_HPP_
