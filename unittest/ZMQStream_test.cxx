/**
 * @file ZmqStream_test.cxx Test sending and receiveing on a STREAM ZMQ socket for raw TCP
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <zmq_addon.hpp> // DISCLAIMER - different library required for multipart messages

#define BOOST_TEST_MODULE ZMQStream_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <vector>

using namespace dunedaq::lbrulibs;

BOOST_AUTO_TEST_SUITE(ZMQStream_test)

BOOST_AUTO_TEST_CASE(SendReceiveTest)
{
  bool m_sender_connected{false};
  bool m_receiver_connected{false};
  zmq::context_t  m_context;
  zmq::socket_t m_publisher{m_context, zmq::socket_type::stream};
  zmq::socket_t m_receiver{m_context, zmq::socket_type::stream};
  std::string m_ZMQLink_sourceLink = "tcp://127.0.0.1:5556";
  std::chrono::milliseconds m_poller_timeout{10000};

  m_sender.connect(m_ZMQLink_sourceLink);
  m_sender_connected = true;
  m_receiver.bind(m_ZMQLink_sourceLink);
  m_receiver_connected = true;

  BOOST_REQUIRE(m_sender_connected);
  BOOST_REQUIRE(m_receiver_connected);

  std::vector<char> test_data{ 'T', 'E', 'S', 'T' };
  std::array<zmq::const_buffer, 2> multipart = {
        m_sender.identity,
        test_data
    };

  zmq::pollitem_t items[] = {{static_cast<void*>(m_receiver),0,ZMQ_POLLIN,0}};
  std::vector<zmq::message_t> recv_msgs;
  zmq::poll (&items [0],1,m_poller_timeout);

  /*
  // less simple but more coherent way as in: https://github.com/zeromq/cppzmq/blob/master/examples/pubsub_multithread_inproc.cpp
  m_sender.send(m_sender.identity, m_sender.identity.size(), zmq::send_flags::sndmore);
  m_sender.send(test_data.data(), test_data.size());
  */

  // simple but less convenient way as in: https://github.com/zeromq/cppzmq/blob/master/examples/multipart_messages.cpp 
  zmq::send_multipart(m_sender, multipart)
  // FIX ME - receive the first empty message from socket connection being established and discard it
  if (items[0].revents & ZMQ_POLLIN){
    auto recvd = zmq::recv_multipart(m_receiver, std::back_inserter(recv_msgs));
  }

  BOOST_REQUIRE_EQUAL(recv_msgs[1].size(), 4);
  BOOST_REQUIRE_EQUAL(recv_msgs[1][0], 'T');
  BOOST_REQUIRE_EQUAL(recv_msgs[1][1], 'E');
  BOOST_REQUIRE_EQUAL(recv_msgs[1][2], 'S');
  BOOST_REQUIRE_EQUAL(recv_msgs[1][3], 'T');

  // FIX ME - test the poller timeout
}

BOOST_AUTO_TEST_SUITE_END()