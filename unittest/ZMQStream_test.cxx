/**
 * @file ZmqStream_test.cxx Test sending and receiveing on a STREAM ZMQ socket for raw TCP
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <zmq.hpp> // DISCLAIMER - different library required for multipart messages

#define BOOST_TEST_MODULE ZMQStream_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(ZMQStream_test)

BOOST_AUTO_TEST_CASE(SendReceiveTest)
{
  bool m_sender_connected{false};
  bool m_receiver_connected{false};
  zmq::context_t  m_context;
  zmq::socket_t m_sender{m_context, zmq::socket_type::stream};
  zmq::socket_t m_receiver{m_context, zmq::socket_type::stream};
  std::string m_ZMQLink_sourceLink = "tcp://127.0.0.1:5556";
  std::chrono::milliseconds m_poller_timeout{10000};

  m_sender.connect(m_ZMQLink_sourceLink);
  m_sender_connected = true;
  m_receiver.bind(m_ZMQLink_sourceLink);
  m_receiver_connected = true;

  BOOST_REQUIRE(m_sender_connected);
  BOOST_REQUIRE(m_receiver_connected);

  std::string test_data{"TEST"};

  zmq::pollitem_t items[] = {{static_cast<void*>(m_receiver),0,ZMQ_POLLIN,0}};
  zmq::poll (&items [0],1,m_poller_timeout);

  if (ZMQ_POLLIN){
    zmq::message_t msg;
    auto identity_recvd = m_receiver.recv(&msg); //receive sender ID
    BOOST_REQUIRE(identity_recvd != 0);
    m_receiver.recv(&msg); //receive empty frame from sender
    std::string empty = std::string(static_cast<char*>(msg.data()), msg.size()); 
    BOOST_REQUIRE_EQUAL(empty, "");


    zmq::message_t replyID;
    auto repID = m_sender.recv(&replyID); //receive receiver ID
    BOOST_REQUIRE(repID != 0);
    m_sender.recv(&msg); //receive empty frame from recceiver

    // Prepare routing message
    zmq::message_t packetID(sizeof(replyID));
    packetID.copy(&replyID); //make a massage using receiver's ID to route to it

    // How to retrieve socket ID after this handshake if needed:
    /*
    char sockID [256];
    size_t sockID_size = sizeof(sockID);
    zmq_getsockopt(m_sender,ZMQ_IDENTITY,sockID,&sockID_size);
    zmq::message_t packetID(sizeof(sockID));
    memcpy(packetID.data(),sockID,sockID_size); 
    */
   
    // Prepare data message 
    zmq::message_t packet(test_data.size());
    memcpy(packet.data(),test_data.data(),test_data.size());
    m_sender.send(packetID,ZMQ_SNDMORE);
    m_sender.send(packet);

    auto recvd = m_receiver.recv(&msg); //Receive routing frame
    BOOST_REQUIRE(recvd != 0);
    auto recvd2 = m_receiver.recv(&msg); //Receive data
    BOOST_REQUIRE(recvd2 != 0);
    BOOST_REQUIRE_EQUAL(msg.size(), 4);
    std::string received = std::string(static_cast<char*>(msg.data()), msg.size());
    BOOST_REQUIRE_EQUAL(received, "TEST");
  }
  // FIX ME - test the poller timeout
}

BOOST_AUTO_TEST_SUITE_END()
