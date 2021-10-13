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
#include "appfwk/app/Nljs.hpp"
#include "nlohmann/json.hpp"
#include "readout/NDReadoutTypes.hpp"
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
  
  // Init
  app::ModInit reader_init_data;
  app::QueueInfo reader_output_queue_info{"reader_output", "output", "out"};
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
  pacman_card_reader->execute_command("conf", reader_conf_json);
  
  // Start
  pacman_card_reader->execute_command("start");  
   

  // Send message  
  zmq::message_t packet(sizeof(message));
  memcpy(packet.data(),message,sizeof(message));
  m_publisher.send(packet);

  // check if data is in the sink somehow...


  // Stop
  pacman_card_reader->execute_command("stop");

  // Scrap
  pacman_card_reader->execute_command("scrap");
  
}

BOOST_AUTO_TEST_SUITE_END()
