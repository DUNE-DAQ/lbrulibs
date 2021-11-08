/**
 * @file PacmanFrame_test.cxx PACMAN message struct and frame class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "detdataformats/pacman/PACMANFrame.hpp"
#include <string>

#define BOOST_TEST_MODULE PacmanFrame_test // NOLINT

#include "boost/test/unit_test.hpp"


BOOST_AUTO_TEST_SUITE(PacmanFrame_test)

BOOST_AUTO_TEST_CASE(PacmanHeader_Methods)
{
  dunedaq::detdataformats::pacman::PACMANFrame::PACMANMessageHeader header;
  
  header.unix_ts = 0x0;
  header.words = 0x1;

  BOOST_REQUIRE_EQUAL(header.unix_ts, 0x0);
  BOOST_REQUIRE_EQUAL(header.words, 0x1);

  //Here we could test all the "get" functions, but for us they take a message as an argument... Perhaps write/take an example message to cast?
  //Message received: b'D\xb0D?a\x00\x01\x00D\x01\xd0\x1f%\x02\x00\x00/\x00@\x00\x00\x00\x00@' //this is wrong - some get interpreted by python print()
  //0x44 0xb0 0x44 0x3f 0x61 0x0 0x1 0x0 0x44 0x1 0xd0 0x1f 0x25 0x2 0x0 0x0 0x2f 0x0 0x40 0x0 0x0 0x0 0x0 0x40 //this is wrong - wrong order of bytes!
  //to get 1631536304 the order should be 0x61 0x3f 0x44 0xb0 in the timestamp
  //trying to correct the order
  //type ... timestamp ... empty word count empty ... word type... word index ... word timestamp ... data (whatever)
  //0x44 ... 0x61 0x3f 0x44 0xb0 ... 0x0 0x1 0x0 ... 0x44 ... 0x1 ... 0x2 0x25 0x1f 0xd0 ... 0x0 0x0 0x2f 0x40 0x40 0x0 0x0 0x0 0x0 0x40
  //Decoded as (('DATA', 1631536304, 1), [('DATA', 1, 35987408, b'/\x00@\x00\x00\x00\x00@')])

  //std::string message = "0x44\xb0\x44\x3f\x61\x0\x1\x0\x44\x1\xd0\x1f\x25\x2\x0\x0\x2f\x0\x40\x0\x0\x0\x0\x40";
  uint32_t message[6] = {0x3f44b044,0x00010061,0x1fd00144,0x00000225,0x0040002f,0x40000000};
  auto frame = reinterpret_cast<const dunedaq::detdataformats::pacman::PACMANFrame*>(&message);
  
  BOOST_REQUIRE_EQUAL(frame->get_msg_header((void *)&message)->unix_ts,1631536304);
  BOOST_REQUIRE_EQUAL(frame->get_msg_header((void *)&message)->words,1);
  
  dunedaq::detdataformats::pacman::PACMANFrame::PACMANMessageWord* theWord = frame->get_msg_word((void *)&message,  0);

  BOOST_REQUIRE_EQUAL(theWord->data_word.type,0x44);
  BOOST_REQUIRE_EQUAL(theWord->data_word.channel_id,0x1);
  BOOST_REQUIRE_EQUAL(theWord->data_word.receipt_timestamp,35987408);

}

BOOST_AUTO_TEST_SUITE_END()
