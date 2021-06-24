# lbrulibs - Near Detecor Low Bandwidth Readout Unit software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE-ND Upstream DAQ Low Bandwidth Readout Unit.

*Under development - not currently working*

## Building

For the readout dependencies, only the DUNEdaq packages are required.

To use the ND-LAr data generator you will need python packages:
- larpix-control - tested on version 3.4.0
- pyzmq - tested on version 18.1.1

## ND-LAr: Configure the PACMAN card
Confugiration steps:
   1. TBD

## ND-GAr: TBD
Confugiration steps:
   1. TBD

## SAND: TBD
Confugiration steps:
   1. TBD

## Examples
In one terminal, launch a fake pacman emaulation by navigating to the test folder and running:

    pacman-generator.py example-pacman-data.h5

To test if the generator is working properly a simple python based ZMQ readout script is provided in the scripts folder:

    python-readout.py

*Not implemented yet*
To test readout through the framework in a separate terminal launch a readout emaulation via:

    daq_application -c stdin://sourcecode/readout/test/pacmanreadout-commands.json
    
Then start typing commands as follows.

## Next development steps:
   1. Create PACMANFrame.hpp based on unpacking functions from LArPix
   2. Make a payload type based on the frame
   3. Create tests to pass the payload through the plugin
   4. Create a request handler for interfacing with Data Selection
   5. Build mechanism for writing data to HDF5
   6. Scale to many ZMQ links and other subdetectors
