/*
 * @file ZmqLink_test.cxx Test sending on a publisher ZMQ socket and receiveing on a ZMQLink from the plugin
 *
 *  * This is part of the DUNE DAQ Application Framework, copyright 2021.
 *  * Licensing/copyright details are in the COPYING file that you should have
 *  * received with this code.
 *     
*/

#include "zmq.hpp"

#define BOOST_TEST_MODULE ZMQPubSub_test // NOLINT

#include "boost/test/unit_test.hpp"
//#include "ZMQLinkConcept.hpp"
//#include "CreateZMQLink.hpp"
#include "nlohmann/json.hpp"

#include <string>
#include <vector>

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

  // FIX ME - get args from json somehow...
  nlohmann::json args;
  // FIX ME - populate the args 
  // example: args["connection_string"] = "some string";
 
  /*
  
  static constexpr size_t m_queue_capacity = 1000000;
  std::string target = "pacman_0"; 
  m_zmqlink[0] = createZMQLinkModel(target);
  if (m_zmqlink[0] == nullptr) {
    ers::fatal(InitializationError(ERS_HERE, "CreateZMQLink failed to provide an appropriate model for queue!"));
  }
  m_zmqlink[0]->init(args, m_queue_capacity); //FIX ME - need args!
  m_zmqlink[0]->conf(args); //FIX ME - need args!
  m_zmqlink[0]->start(args); //FIX ME - need args!
  
  */ 

  // Send message  
  zmq::message_t packet(sizeof(message));
  memcpy(packet.data(),message,sizeof(message));
  m_publisher.send(packet);

  // check if data is in the sink somehow...
  // sink_t is inside the m_zmqlink, alongside m_sink_queue, maybe can retrieve some things about them somehow
  // if we can access the sinkand other variables inside m_zmqlink, could do things such as BOOST_REQUIRE(m_sink_is_set) for starters
}

BOOST_AUTO_TEST_SUITE_END()
