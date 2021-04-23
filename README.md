# lbrulibs - Near Detecor Low Bandwidth Readout Unit software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE-ND Upstream DAQ Low Bandwidth Readout Unit.

*Under development - not currently working*

## Building

For the readout dependencies, only the DUNEdaq packages are required.
To use the ND-LAr data generator you will need python packages:
- larpix-control - tested on version 3.4.0
- pyzmq - tested on version 18.1.1

## ND-LAr: Configure the PACMAN card
Please ensure the following:
   1. TBD

## ND-GAr: TBD
Please ensure the following:
   1. TBD

## SAND: TBD
Please ensure the following:
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
   1. Make ZMQLinkModel to handle sockets and data collection 
   2. Make PacmanCardWrapper to make instances of the plugin for testing
   3. Implement parser operations
   4. Write test apps
   5. Create a set packet format analogous to felix (ask larpix to make a cpp version of their libraries)?
   6. Scale to many ZMQ links
