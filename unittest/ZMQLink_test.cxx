/*
 * @file ZmqLink_test.cxx Test sending on a publisher ZMQ socket and receiveing on a ZMQLink from the plugin
 *
 *  * This is part of the DUNE DAQ Application Framework, copyright 2021.
 *  * Licensing/copyright details are in the COPYING file that you should have
 *  * received with this code.
 *     
*/

#include "zmq.hpp"

#define BOOST_TEST_MODULE ZMQLink_test // NOLINT

#include "boost/test/unit_test.hpp"
#include "appfwk/DAQModule.hpp"
#include "lbrulibs/pacmancardreader/Nljs.hpp"
#include "lbrulibs/pacmancardreaderinfo/InfoNljs.hpp"
#include "appfwk/app/Nljs.hpp"
#include "nlohmann/json.hpp"
#include "ndreadoutlibs/NDReadoutTypes.hpp"
#include <string>
#include <vector>

using namespace dunedaq::appfwk;
using namespace dunedaq::lbrulibs;

BOOST_AUTO_TEST_SUITE(ZMQLink_test)

BOOST_AUTO_TEST_CASE(LinkTest)
{
  bool m_publisher_connected{false};
  zmq::context_t  m_context;
  zmq::socket_t m_publisher{m_context, zmq::socket_type::pub};
  std::string m_ZMQLink_sourceLink = "tcp://127.0.0.1:5556";

  m_publisher.bind(m_ZMQLink_sourceLink);
  m_publisher_connected = true;

  BOOST_REQUIRE(m_publisher_connected);

  uint32_t message[6] = {0x3f44b044,0x00010061,0x1fd00144,0x00000225,0x0040002f,0x40000000};
  //Decoded as (('DATA', 1631536304, 1), [('DATA', 1, 35987408, b'/\x00@\x00\x00\x00\x00@')])

  // Create plugin module
  std::shared_ptr<DAQModule> pacman_card_reader = make_module("PacmanCardReader", "reader");
  
  std::map<std::string, QueueConfig> queue_map;
  queue_map["pacman_link_0"] = QueueConfig{ QueueConfig::kStdDeQueue, 10000 };
  QueueRegistry::get().configure(queue_map);

  // Init
  app::ModInit reader_init_data;
  app::QueueInfo reader_output_queue_info{"pacman_link_0", "output", "output"};
  reader_init_data.qinfos.push_back(reader_output_queue_info); 
  nlohmann::json reader_init_json;
  to_json(reader_init_json, reader_init_data);
  pacman_card_reader->init(reader_init_json);

  // Conf
  pacmancardreader::Conf reader_config;
  //To change anyhing in the conf do like
  //reader_config.some_property = "option";
  nlohmann::json reader_conf_json;
  to_json(reader_conf_json, reader_config);
  pacman_card_reader->execute_command("conf", "INITIAL", reader_conf_json);
  
  // Start
  pacman_card_reader->execute_command("start", "CONFIGURED");  
   
  usleep(10000); //small sleep required to allow startup to complete
  
  // Send message  
  zmq::message_t packet(sizeof(message));
  memcpy(packet.data(),message,sizeof(message));
  m_publisher.send(packet);

  usleep(10000); //small sleep required to allow propagation of message

  // check if data has been received using opmon interface
  dunedaq::opmonlib::InfoCollector ci;
  pacman_card_reader->get_info(ci, 2);
  const nlohmann::json theInfo = ci.get_collected_infos();

  try{
    int numPackets = theInfo.at("__properties").at("dunedaq.lbrulibs.pacmancardreaderinfo.ZMQLinkInfo").at("__data").at("num_packets_received");
    BOOST_REQUIRE(numPackets==1);
  }
  catch(std::exception& e){
    BOOST_ERROR("Unable to Read JSON: " + std::string(e.what()));
  }

  // Stop
  pacman_card_reader->execute_command("stop", "RUNNING");

  // Scrap
  //pacman_card_reader->execute_command("scrap", "CONFIGURED");
  
}

BOOST_AUTO_TEST_SUITE_END()
