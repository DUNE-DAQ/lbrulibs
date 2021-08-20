/**
 * @file ZmqPubSub_test.cxx Test sending on a publisher ZMQ socket and receiveing on a subscriber ZMQ socket
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "zmq.hpp"

#define BOOST_TEST_MODULE ZMQPubSub_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <vector>

using namespace dunedaq::lbrulibs;

BOOST_AUTO_TEST_SUITE(ZMQPubSub_test)

BOOST_AUTO_TEST_CASE(SendReceiveTest)
{
  bool m_publisher_connected{false};
  bool m_subscriber_connected{false};
  zmq::context_t  m_context;
  zmq::socket_t m_publisher{m_context, zmq::socket_type::pub};
  zmq::socket_t m_subscriber{m_context, zmq::socket_type::sub};
  std::string m_ZMQLink_sourceLink = "tcp://127.0.0.1:5556";
  std::chrono::milliseconds m_poller_timeout{10000};

  m_publisher.bind(m_ZMQLink_sourceLink);
  m_publisher_connected = true;
  m_subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
  m_subscriber.connect(m_ZMQLink_sourceLink);
  m_subscriber_connected = true;
  //m_subscriber.setsockopt(ZMQ_SUBSCRIBE, "");

  BOOST_REQUIRE(m_publisher_connected);
  BOOST_REQUIRE(m_subscriber_connected);

  std::vector<char> test_data{ 'T', 'E', 'S', 'T' };
  
  zmq::pollitem_t items[] = {{static_cast<void*>(m_subscriber),0,ZMQ_POLLIN,0}};
  zmq::message_t msg;
  zmq::poll (&items [0],1,m_poller_timeout);

  m_publisher.send(test_data.data(), test_data.size(), "");
  if (items[0].revents & ZMQ_POLLIN){
    auto recvd = m_subscriber.recv(&msg);
  }

  BOOST_REQUIRE_EQUAL(msg.data.size(), 4);
  BOOST_REQUIRE_EQUAL(msg.data[0], 'T');
  BOOST_REQUIRE_EQUAL(msg.data[1], 'E');
  BOOST_REQUIRE_EQUAL(msg.data[2], 'S');
  BOOST_REQUIRE_EQUAL(msg.data[3], 'T');

  // FIX ME - test the poller timeout
  /*
  m_subscriber.setsockopt(ZMQ_UNSUBSCRIBE, "", 0);
  zmq::poll (&items [0],1,std::chrono::milliseconds(100));
  m_publisher.send(test_data.data(), test_data.size(), "");
  BOOST_REQUIRE_EXCEPTION(
    if (items[0].revents & ZMQ_POLLIN){auto recvd = m_subscriber.recv(&msg);},
    zmq::ZMQError);
  */
}

BOOST_AUTO_TEST_SUITE_END()