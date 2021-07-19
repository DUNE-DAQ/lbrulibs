cmake_minimum_required(VERSION 3.12)
project(lbrulibs VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(dataformats REQUIRED)
find_package(readout REQUIRED)
find_package(ipm REQUIRED)

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

##############################################################################
# Dependency sets
set(DUNEDAQ_DEPENDENCIES readout::readout dataformats::dataformats ipm::ipm)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")


##############################################################################
# Main library
daq_add_library(LINK_LIBRARIES appfwk::appfwk ${DUNEDAQ_DEPENDENCIES})

##############################################################################
# Plugins
daq_add_plugin(PacmanCardReader duneDAQModule LINK_LIBRARIES appfwk::appfwk ${DUNEDAQ_DEPENDENCIES} lbrulibs)

##############################################################################
# Integration tests
#daq_add_application(lbrulibs_test_cardwrapper test_pacmancardwrapper_app.cxx TEST LINK_LIBRARIES lbrulibs)
#daq_add_application(lbrulibs_test_zmqlinkhandler test_zmqlinkhandler_app.cxx TEST LINK_LIBRARIES lbrulibs)
#daq_add_application(lbrulibs_test_zmqlink_to_file test_zmqlink_to_file_app.cxx TEST LINK_LIBRARIES lbrulibs)

##############################################################################
# Test configurations
file(COPY test/pacmanreadout-commands.json DESTINATION test)

##############################################################################
# Installation
daq_install()
