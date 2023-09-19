# lbrulibs - Near Detecor Low Bandwidth Readout Unit software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE-ND Upstream DAQ Low Bandwidth Readout Unit.

As of DUNE-DAQ version 2.11 the default functionality uses ZMQ STREAM sockets, which can accept any type of TCP messages. For prior ZMQ PUB-SUB setup see further.

If you're a first time user of DUNE DAQ framework see the [docs for daq-buildtools](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-buildtools/) to learn how to perform initial setup.

If you're a developer interested in sending data to be received by lbrulibs see [socket-implementation](socket-implementation.md) for the connection setup/pattern.

## Dependencies

- For readout dependencies, only what is included in the DUNEdaq working environment is required.

- For fake pacman card emulation (data generator and automated integration test) python libraries needed are: bitarray, bidict, zmq. These can be installed via pip or taken from the pypi-repo.

## ND-LAr: Examples with PACMAN data snapshots
In one terminal, launch a fake pacman emulation by navigating to the test folder and running:

    python pacman-generator-RAW.py --input_file example-pacman-data.h5

The script is able generate messages in a number of different ways. The interval of time between each message being sent, for example,
is a configurable parameter. For more details run:

    python pacman-generator-RAW.py --help

To test if the generator is working properly a simple python based ZMQ readout script is provided in the scripts folder:

    python python-readout-RAW.py

This will receive the packets sent out by the generator over ZMQ and print out some useful debug information.

To test readout through the DUNE-DAQ readout framework, start a separate terminal on the same machine and launch the test application via:

    daq_application -n appNameofYourChoice -c <path_to_source>/lbrulibs/python/lbrulibs/fake_NDreadout.json
    
Then cycle through the states by typing 'init', 'conf' and 'start'. To stop the run issue the 'stop' command. To record data to a file
issue the 'record' command. Note that this will store a data dump from the luminosity buffer should its occupancy rise above 80%. The stored
data are 'raw' and not subject to any trigger selection.

Note - due to the need for a more configurable fake trigger implementation in readout this test will produce numerous warnings for failed trigger
requests. These can be safely ignored.

To use nanorc instead (preffered for testing) use:

    nddaqconf_gen --host-ru localhost -o . --number-of-data-producers 1 --frontend-type pacman --trigger-window-before-ticks 2500000 --trigger-window-after-ticks 2500000 --trigger-rate-hz 1.0 --enable-raw-recording mdapp_4proc_pacman_1Hz_pt1second_mode3

to generate a config and then:

    nanorc mdapp_4proc_pacman_1Hz_pt1second_mode3

to run it. With run commands: boot (insance-name), init, conf, start (run number), resume, (here receive data), stop, scrap, exit

## ND-GAr: TBD
Configuration steps:
   1. TBD

## SAND: TBD
Configuration steps:
   1. TBD

## Testing
To perform unit tests simply use the --unittest build option:

    dbt-build.py --unittest

To run a full integration test of the ND-LAr readout with a simulated data source use (in integtest folder):

    pytest -s test_pacman-raw.py --frame-file $PWD/frames.bin

## ZMQ PUB-SUB (soon to be deprecated)
To regain the use of a ZMQ subscriber socket in lbrulibs ND-LAr readout:
   1. In PacmanCardReader.cpp on line 22 set bool usePUBSUB = 1;
   2. Rebuild using dbt-build
This functionality is currently planned to be removed in daq framework version 3.X, moving solely to raw-TCP readout via "STREAM" sockets - this will lead to universality in connection types for all subdetectors. Be aware that one unittest will fail when you make this change (since it relies on STREAM sockets). 

Python scripts for testing as well as a full integration test for this are still supplied - named the same as the raw-TCP versions but without the "RAW" suffix.

## Next development steps:
   1. Scale to many ZMQ links and other subdetectors.
