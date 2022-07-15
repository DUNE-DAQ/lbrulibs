cmake_minimum_required(VERSION 3.12)
project(lbrulibs VERSION 1.2.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(ers REQUIRED)
find_package(detdataformats REQUIRED)
find_package(readoutlibs REQUIRED)
find_package(ndreadoutlibs REQUIRED)
find_package(ipm REQUIRED)
find_package(iomanager REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

daq_codegen(pacmancardreader.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen(pacmancardreaderinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

##############################################################################
# Dependency sets
set(DUNEDAQ_DEPENDENCIES appfwk::appfwk readoutlibs::readoutlibs ndreadoutlibs::ndreadoutlibs detdataformats::detdataformats ipm::ipm iomanager::iomanager)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")


##############################################################################
# Main library
daq_add_library(LINK_LIBRARIES ${DUNEDAQ_DEPENDENCIES})

##############################################################################
# Plugins
daq_add_plugin(PacmanCardReader duneDAQModule LINK_LIBRARIES ${DUNEDAQ_DEPENDENCIES} lbrulibs)

##############################################################################
# Integration tests
#daq_add_application(lbrulibs_test_something test_pacmansomething_app.cxx TEST LINK_LIBRARIES lbrulibs)

##############################################################################
# Test configurations
file(COPY test/pacmanreadout-commands.json DESTINATION test)

##############################################################################
# Unit Tests
daq_add_unit_test(ZMQPubSub_test                LINK_LIBRARIES lbrulibs)
daq_add_unit_test(ZMQStream_test                LINK_LIBRARIES lbrulibs)
daq_add_unit_test(PacmanFrame_test              LINK_LIBRARIES lbrulibs)
##daq_add_unit_test(ZMQLink_test                  LINK_LIBRARIES lbrulibs)
#daq_add_unit_test(STREAMLink_test               LINK_LIBRARIES lbrulibs)

##############################################################################
# Installation
daq_install()
