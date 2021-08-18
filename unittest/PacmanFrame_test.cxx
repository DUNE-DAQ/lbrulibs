/**
 * @file PacmanFrame_test.cxx PACMAN message struct and frame class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "dataformats/pacman/PACMANFrame.hpp"

#define BOOST_TEST_MODULE PacmanFrame_test // NOLINT

#include "boost/test/unit_test.hpp"

using namespace dunedaq::dataformats;

BOOST_AUTO_TEST_SUITE(PacmanFrame_test)

BOOST_AUTO_TEST_CASE(PacmanHeader_Methods)
{
  PACMANFrame frame;
  PACMANFrame::PACMANMessageHeader header;

  header.type = 0x44;
  header.unix_ts = 0x0;
  header.words = 0x1;

  BOOST_REQUIRE_EQUAL(header.type, 0x44);
  BOOST_REQUIRE_EQUAL(header.unix_ts, 0x0);
  BOOST_REQUIRE_EQUAL(header.words, 0x1);

  // Here we could test all the "get" functions, but for us they take a message as an argument... Perhaps write/take an example message to cast?
  //void message = D\x89\xde\x1ca\x00\x01\x00D\x01\xd0\x1f%\x02\x00\x00/\x00@\x00\x00\x00\x00@;
  // Decoded as (('DATA', 1629281929, 1), [('DATA', 1, 35987408, b'/\x00@\x00\x00\x00\x00@')])
  //BOOST_REQUIRE_EQUAL(frame->get_msg_header(message)->unix_ts),0x89)
}

BOOST_AUTO_TEST_SUITE_END()