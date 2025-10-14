import pytest
import urllib.request
import os
import copy
import re
import math
import multiprocessing 

import socket

import integrationtest.data_file_checks as data_file_checks
import integrationtest.log_file_checks as log_file_checks
import integrationtest.data_classes as data_classes

from rich.progress import Progress, SpinnerColumn, BarColumn, TextColumn


lbrulibs_dir=os.path.realpath(os.path.dirname(__file__) + "/../")
# Set up the message sender here:
import time
import sys

sys.path.insert(1, f"{lbrulibs_dir}/scripts")

import larpixtools
import zmq


pytest_plugins="integrationtest.integrationtest_drunc"
# Values that help determine the running conditions
number_of_data_producers = 2
data_rate_slowdown_factor = 1  # 10 for ProtoWIB/DuneWIB
run_duration = 20  # seconds
readout_window_time_before = 1000
readout_window_time_after = 1001

# Default values for validation parameters
expected_number_of_data_files = 1
check_for_logfile_errors = True
expected_event_count = run_duration
expected_event_count_tolerance = 4


pacman_frag_params = {
    "fragment_type_description": "PACMAN",
    "fragment_type": "PACMAN",
    "hdf5_source_subsystem": "Detector_Readout",
    "expected_fragment_count": number_of_data_producers,
    "min_size_bytes": 80,
    "max_size_bytes": 1048656,
}


ignored_logfile_problems = {
    "-controller": [
        "Worker with pid \\d+ was terminated due to signal",
        "Connection '.*' not found on the application registry",
    ],
    "local-connection-server": [
        "errorlog: -",
        "Worker with pid \\d+ was terminated due to signal",
    ],
    "log_.*_minimal_": ["connect: Connection refused"],
}

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(("127.0.0.1", 0))  # Bind to any free port
    # print(s.getsockname())
    # not sure why, doing socket + 1 works best....
    data_stream_sock_id=s.getsockname()[1]+1
    data_stream_socket = f"tcp://{s.getsockname()[0]}:{s.getsockname()[1]+1}"
    s.close()


# The next three variable declarations *must* be present as globals in the test
# file. They're read by the "fixtures" in conftest.py to determine how
# to run the config generation and nanorc
conf_dict = data_classes.drunc_config()
conf_dict.op_env = "integtest"
conf_dict.session = "minimal"
conf_dict.use_fakedataprod = False
conf_dict.dro_map_config.n_streams = number_of_data_producers


# conf_dict.config_substitutions.append(
#     data_classes.config_substitution(
#         obj_class="Service",
#         obj_id="dataRequests",
#         updates={"port": f"{data_stream_sock_id}"}
#     )
# )

conf_dict.config_substitutions.append(
    data_classes.config_substitution(
        obj_class="PACMANDataReaderConf",
        obj_id="def-pac-receiver-conf",
        updates={"emulation_mode": "0"}
    )
)


# conf_dict.config_substitutions.append(
#     data_classes.config_substitution(
#         obj_id="dummy-detector",
#         obj_class="DetectorConfig",
#         updates={"clock_speed_hz": 1000000000}, # FakeDataProd uses nanoseconds as its timestamps
#     )
# )
object_databases = ["config/daqsystemtest/integrationtest-objects.data.xml"]


hsi_frag_params = {
    "fragment_type_description": "HSI",
    "fragment_type": "Hardware_Signal",
    "hdf5_source_subsystem": "HW_Signals_Interface",
    "expected_fragment_count": 0,
    "min_size_bytes": 72,
    "max_size_bytes": 100,
}
ignored_logfile_problems = {
    "-controller": [
        "Worker with pid \\d+ was terminated due to signal",
        "Connection '.*' not found on the application registry",
    ],
    "connectivity-service": [
        "errorlog: -",
        "Worker with pid \\d+ was terminated due to signal",
    ],
    "log_.*_minimal_": ["connect: Connection refused"],
}

# conf_dict = data_classes.drunc_config()
# conf_dict.dro_map_config.n_streams = number_of_data_producers
# conf_dict.op_env = "integtest"
# conf_dict.session = "minimal"
# conf_dict.tpg_enabled = False

detid_ND_LAr = 32
number_of_apps = 1


confgen_arguments = {"MinimalSystem": conf_dict}
# The commands to run in nanorc, as a list
nanorc_command_list = (
    "boot conf start --run-number 101 wait 1 enable-triggers wait ".split()
    + [str(run_duration)]
    + "disable-triggers wait 2 drain-dataflow wait 2 stop-trigger-sources stop scrap terminate".split()
)


def hdf5ToPackets(datafile): 
    print("Reading from:",datafile)
    packets = larpixtools.from_file(datafile)['packets'] #read from HDF5 file
    print("Separating into messages based on timestamp packets...")
    msg_breaks = [i for i in range(len(packets)) if packets[i].packet_type == 0 or i == len(packets)-1] #find the timestamp packets which signify message breaks
    msg_packets = [packets[i:j] for i,j in zip(msg_breaks[:-1], msg_breaks[1:])] #separate into messages
    msgs = [larpixtools.format(p, msg_type='DATA') for p in msg_packets]
    print("Extracting headers and words from messages...")
    word_lists = [larpixtools.parse_msg(p)[1] for p in msgs] #retrieve lists of words from each message
    print("Read complete. PACMAN style messages prepared.")
    return word_lists



def sender(_data_server, word_lists,ready_event):
    
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
        print("Binding sockets...")
        

        id = 0
        while id == 0:
            try:
                data_socket.connect(_data_server)
                id = data_socket.recv()
                ready_event.set()
                
            except Exception as e:
                # print(f"\n{e}")
                time.sleep(1)

        print(f"Connected to {_data_server}")
        print('Initialising...')
        time.sleep(1)

        print('Sending PACMAN data.')
        for i in word_lists:
            data_socket.send_multipart([id,larpixtools.format_msg('DATA',i)])
            time.sleep(0.1)
    except:
        raise
    finally:   
        data_socket.close()
        ctx.destroy()


# data_socket = 'tcp://127.0.0.1:55561'
# data_file = f"{lbrulibs_dir}/test/example-pacman-data.h5"

# word_lists = hdf5ToPackets(data_file)
# print("Starting PACMAN card(s)")
# process = multiprocessing.Process(target=sender,args=[data_socket,word_lists])
# process.daemon = True
# process.start()

@pytest.fixture(scope="session")
def pacman_sender():
    """
    Pytest fixture to ensure the PACMAN sender process is running before tests.
    """
    # data_socket = 'tcp://127.0.0.5102'
    data_file = f"{lbrulibs_dir}/test/example-pacman-data.h5"

    # Prepare word lists from the data file
    word_lists = hdf5ToPackets(data_file)

    ready_event = multiprocessing.Event()


    print("Starting PACMAN card(s) simulation...")
    process = multiprocessing.Process(target=sender, args=[data_stream_socket, word_lists, ready_event])
    process.daemon = True
    process.start()

    # Wait for cards to connect
    timeout = 300
    start_time = time.time()
    with Progress(
        TextColumn("[bold red]Timeout: "),
        SpinnerColumn(),
        BarColumn(),
        TextColumn("{task.description}"),
        transient=True,
    ) as progress:
        task = progress.add_task("[cyan]Waiting for PACMAN sender to connect...", total=timeout)
        while not ready_event.is_set() and time.time() - start_time < timeout:
            progress.update(task, advance=1, description=f"[cyan]{int(time.time() - start_time)}s/{timeout}s elapsed")
            time.sleep(1)

    if not ready_event.is_set():
        process.terminate()
        process.join()
        pytest.fail("PACMAN sender process failed to connect within the timeout period.")

    yield process  # Provide the process to tests if needed


    # Cleanup after tests
    print("Testing done!")
    process.terminate()
    process.join()

# The tests themselves
def test_nanorc_success(run_nanorc, pacman_sender):
    # Check that nanorc completed correctly
    assert run_nanorc.completed_process.returncode == 0

def test_log_files(run_nanorc):
    local_check_flag = check_for_logfile_errors

    if local_check_flag:
        # Check that there are no warnings or errors in the log files
        assert log_file_checks.logs_are_error_free(
            run_nanorc.log_files, True, True, ignored_logfile_problems
        )


def test_data_files(run_nanorc):
    local_expected_event_count = expected_event_count
    local_event_count_tolerance = expected_event_count_tolerance
    # frag_params=wib1_frag_hsi_trig_params # ProtoWIB
    # frag_params=wib2_frag_params # DuneWIB
    frag_params = pacman_frag_params # pacman
    current_test = os.environ.get("PYTEST_CURRENT_TEST")
    if "Double" in current_test:
        # frag_params["min_size_bytes"]=72+(464*161) # 161 frames of 464 bytes each with 72-byte Fragment header # ProtoWIB
        # frag_params["max_size_bytes"]=72+(464*161)
        # frag_params["min_size_bytes"]=72+(472*math.ceil(4001/32)) # 126 frames of 472 bytes each with 72-byte Fragment header # DuneWIB
        # frag_params["max_size_bytes"]=72+(472*math.ceil(4001/32))
        frag_params["min_size_bytes"] = 72 + (
            7200 * math.ceil(4001 / 2048)
        )  # 2 frames of 7200 bytes each with 72-byte Fragment header # WIBEth
        frag_params["max_size_bytes"] = 72 + (7200 * (1 + math.ceil(4001 / 2048)))
    fragment_check_list = [frag_params]

    # Run some tests on the output data file
    all_ok = True
    all_ok &= len(run_nanorc.data_files) == expected_number_of_data_files

    for idx in range(len(run_nanorc.data_files)):
        data_file = data_file_checks.DataFile(run_nanorc.data_files[idx])
        all_ok &= data_file_checks.sanity_check(data_file)
        all_ok &= data_file_checks.check_file_attributes(data_file)
        all_ok &= data_file_checks.check_event_count(
            data_file, local_expected_event_count, local_event_count_tolerance
        )
        for jdx in range(len(fragment_check_list)):
            all_ok &= data_file_checks.check_fragment_count(
                data_file, fragment_check_list[jdx]
            )
            all_ok &= data_file_checks.check_fragment_sizes(
                data_file, fragment_check_list[jdx]
            )

    assert all_ok

