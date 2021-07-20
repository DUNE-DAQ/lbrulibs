# lbrulibs - Near Detecor Low Bandwidth Readout Unit software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE-ND Upstream DAQ Low Bandwidth Readout Unit.

## Building

For the readout dependencies, only the DUNEdaq packages are required.

To use the ND-LAr data generator you will need python packages:
- larpix-control - tested on version 3.5.1
- pyzmq - tested on version 18.1.1

## ND-LAr: Examples with PACMAN data snapshots
In one terminal, launch a fake pacman emulation by navigating to the test folder and running:

    pacman-generator.py example-pacman-data.h5

To test if the generator is working properly a simple python based ZMQ readout script is provided in the scripts folder:

    python-readout.py

This will receive the packets sent out by the generator over ZMQ and print out some useful debug information.

To test readout through the DUNE-DAQ readout framework, start a separate terminal on the same machine and launch the test application via:

    daq_application -n appNameofYourChoice -c <path_to_source>/lbrulibs/python/lbrulibs/fake_NDreadout.json
    
Then cycle through the states by typing 'init', 'conf' and 'start'. To stop the run issue the 'stop' command. To record data to a file
issue the 'record' command. Note that this will store a data dump from the luminosity buffer should its occupancy rise above 80%. The stored
data are 'raw' and not subject to any trigger selection.

Note - due to the need for a more configurable fake trigger implementation in readout this test will produce numerous warnings for failed trigger
requests. These can be safely ignored.

## ND-GAr: TBD
Configuration steps:
   1. TBD

## SAND: TBD
Configuration steps:
   1. TBD


## Next development steps:
   1. Verify ability to write triggered data to HDF5 via minidaqapp
   2. Update fake trigger configuration in readout package to enable merge to develop (under discussion with readout developers)
   3. Conclude on ZMQ IPM timeout issue
   4. Scale to many ZMQ links and other subdetectors
