cmake_minimum_required(VERSION 3.12)
project(lbrulibs VERSION 1.0.5)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(detdataformats REQUIRED)
find_package(readout REQUIRED)
find_package(ipm REQUIRED)

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

##############################################################################
# Dependency sets
set(DUNEDAQ_DEPENDENCIES appfwk::appfwk readout::readout detdataformats::detdataformats ipm::ipm)
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
daq_add_unit_test(ZMQLink_test                  LINK_LIBRARIES lbrulibs)

##############################################################################
# Installation
daq_install()
