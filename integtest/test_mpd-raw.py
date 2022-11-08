import pytest
import dfmodules.data_file_checks as data_file_checks
import dfmodules.integtest_file_gen as integtest_file_gen
import integrationtest.log_file_checks as log_file_checks
import integrationtest.config_file_gen as config_file_gen

# Values that help determine the running conditions
number_of_data_producers=1
rate = 1
sleep_time = 0 
sent_data = 100

#delay sending
delay=0

run_duration=int(sent_data*rate) + sleep_time # seconds

# Default values for validation parameters
expected_number_of_data_files=1
check_for_logfile_errors=True
expected_event_count_tolerance=1

wib1_frag_hsi_trig_params={"fragment_type_description": "MPD",
                           "fragment_type": "MPD",
                           "hdf5_source_subsystem": "Detector_Readout",
                           "hdf5_detector_group": "NDLAr_PDS", 
                           "hdf5_region_prefix": "Region",
                           "element_name_prefix": "Element", 
                           "element_number_offset": 0, 
                           "expected_fragment_count": number_of_data_producers,
                           "min_size_bytes": 28, 
                           "max_size_bytes": 3724}

# The next three variable declarations *must* be present as globals in the test
# file. They're read by the "fixtures" in conftest.py to determine how
# to run the config generation and nanorc

# The name of the python module for the config generation
confgen_name="daqconf_multiru_gen"
# The arguments to pass to the config generator, excluding the json
# output directory (the test framework handles that)
detid_NDLAr_PDS = 33 
number_of_apps = 1 
hardware_map_contents = integtest_file_gen.generate_hwmap_file( number_of_data_producers, number_of_apps, detid_NDLAr_PDS )

conf_dict = config_file_gen.get_default_config_dict()
conf_dict["boot"]["op_env"] = "integtest"
conf_dict["trigger"]["trigger_window_before_ticks"] = 30000
conf_dict["trigger"]["trigger_window_after_ticks"] = 30000

confgen_arguments={"MPDSystem": conf_dict}

# The commands to run in nanorc, as a list
nanorc_command_list="integtest-partition boot conf start 101 wait 1 enable_triggers wait ".split() + [str(run_duration)] + " disable_triggers wait 2 stop_run wait 2 scrap terminate".split()

# The tests themselves
def test_nanorc_success(run_nanorc):
    # Check that nanorc completed correctly
    assert run_nanorc.completed_process.returncode==0
def test_log_files(run_nanorc):
    if check_for_logfile_errors:
        # Check that there are no warnings or errors in the log files
        assert log_file_checks.logs_are_error_free(run_nanorc.log_files)
def test_data_file(run_nanorc):
    # Run some tests on the output data file
    assert len(run_nanorc.data_files)==expected_number_of_data_files
    for idx in range(len(run_nanorc.data_files)):
        data_file=data_file_checks.DataFile(run_nanorc.data_files[idx])
        assert data_file_checks.sanity_check(data_file)
        assert data_file_checks.check_event_count(data_file,run_duration,10)
        assert data_file_checks.check_fragment_count(data_file, wib1_frag_hsi_trig_params)

import os
import numpy as np
import argparse
import sys
import struct
from collections import deque
sys.path.insert(1, '../test')
import mpd_helper as mpd
import time
import zmq
import random

# Prepare ports
data = 'tcp://127.0.0.1:5556'

def send_mpd(packets, n_packets):
    try:
        # Set up sockets
        print("Setting up ZMQ sockets...")
        ctx = zmq.Context()
        data_socket = ctx.socket(zmq.STREAM)
        socket_opts = [
            (zmq.LINGER,100),
            (zmq.RCVTIMEO,100),
            (zmq.SNDTIMEO,100)
        ]
        print("Parsing socket options...")
        for opt in socket_opts:
            data_socket.setsockopt(*opt)
        print("Connecting sockets...")
        id = 0
        while id == 0:
            try:
                data_socket.connect(data)
            
                # need to receive two messages to get target ID
                id = data_socket.recv()
                message = data_socket.recv()
            except:
                print("No receiver ready to connect to. Retrying...")
                time.sleep(1) #wait 1s before retrying
                continue

        print('Initialising...')
        time.sleep(1+delay)

        print('Sending ', n_packets, ' MPD messages.')

        # Send messages in intervals based on timestamps
        message_count = 0
        for i in range(n_packets):
            data_socket.send_multipart([id,packets[i]])
            
            message_count += 1
            print("Total messages sent:",message_count)        
            time.sleep(rate);

        print("Sleeping for 10 seconds before exiting...")
        time.sleep(sleep_time)
    except:
        raise
    finally: #cleanup
        data_socket.close()
        ctx.destroy()

print("Starting MPD card(s)")
import multiprocessing

mpd_data = mpd.mpd("../test/example-mpd-data-100events.data", 1, sent_data) 

run_duration=int(mpd_data.num_packets()*rate) + sleep_time  # seconds
expected_event_count=run_duration
print( ' Sending ', mpd_data.num_packets() , ' packets' )
print( ' Number of expected triggers = ' , run_duration )

process = multiprocessing.Process(target=send_mpd,args=[mpd_data.packets, mpd_data.num_packets(),])
process.daemon = True
process.start()
