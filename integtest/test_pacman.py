import pytest
import dfmodules.data_file_checks as data_file_checks
import integrationtest.log_file_checks as log_file_checks
# Values that help determine the running conditions
number_of_data_producers=1
run_duration=60  # seconds
# Default values for validation parameters
expected_number_of_data_files=1
check_for_logfile_errors=True
expected_event_count=run_duration
expected_event_count_tolerance=2
wib1_frag_hsi_trig_params={"fragment_type_description": "Pacman", "hdf5_groups": "NDLArTPC/Region000",
                           "element_name_prefix": "Element", "element_number_offset": 0,
                           "expected_fragment_count": number_of_data_producers,
                           "min_size_bytes": 80, "max_size_bytes": 1048656}
# The next three variable declarations *must* be present as globals in the test
# file. They're read by the "fixtures" in conftest.py to determine how
# to run the config generation and nanorc
# The name of the python module for the config generation
confgen_name="minidaqapp.nanorc.mdapp_multiru_gen"
# The arguments to pass to the config generator, excluding the json
# output directory (the test framework handles that)
confgen_arguments=[ "--host-ru", "localhost", "-o", ".", "-n", str(number_of_data_producers), "--frontend-type", "pacman", "-b", "2500000", "-a", "2500000", "-t", "1.0" ]
# The commands to run in nanorc, as a list
nanorc_command_list="boot init conf start --resume-wait 1 101 wait ".split() + [str(run_duration)] + " stop --stop-wait 1 wait 2 scrap terminate".split()

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
        assert data_file_checks.check_fragment_count(data_file, wib1_frag_hsi_trig_params)
        assert data_file_checks.check_fragment_presence(data_file, wib1_frag_hsi_trig_params)
        assert data_file_checks.check_fragment_size2(data_file, wib1_frag_hsi_trig_params)
        # 26-Oct-2021, KAB: checking the event count doesn't make sense currently, since we
        # start the Pacman generator by hand and there is no guarantee for when we do that...
        #assert data_file_checks.check_event_count(data_file, expected_event_count, expected_event_count_tolerance)

# Set up the message sender here:
import time
import larpix
from larpix.format import pacman_msg_format
import zmq

data_socket = 'tcp://127.0.0.1:5556'
data_file = '../test/example-pacman-data.h5'

def hdf5ToPackets(datafile): 
    print("Reading from:",datafile)
    packets = larpix.format.hdf5format.from_file(datafile)['packets'] #read from HDF5 file
    print("Separating into messages based on timestamp packets...")
    msg_breaks = [i for i in range(len(packets)) if packets[i].packet_type == 4 or i == len(packets)-1] #find the timestamp packets which signify message breaks
    msg_packets = [packets[i:j] for i,j in zip(msg_breaks[:-1], msg_breaks[1:])] #separate into messages
    msgs = [pacman_msg_format.format(p, msg_type='DATA') for p in msg_packets]
    print("Extracting headers and words from messages...")
    word_lists = [pacman_msg_format.parse_msg(p)[1] for p in msgs] #retrieve lists of words from each message
    print("Read complete. PACMAN style messages prepared.")
    return word_lists

def sender_setup(_data_server):
    # Set up sockets
    print("Setting up ZMQ sockets...")
    ctx = zmq.Context()
    data_socket = ctx.socket(zmq.PUB)
    socket_opts = [
        (zmq.LINGER,100),
        (zmq.RCVTIMEO,100),
        (zmq.SNDTIMEO,100)
    ]
    print("Parsing socket options...")
    for opt in socket_opts:
        data_socket.setsockopt(*opt)
    print("Binding sockets...")
    data_socket.bind(_data_server)
    print('Initialising...')
    time.sleep(1)
    return data_socket

def sender(data_socket,word_lists):
    print('Sending PACMAN data.')
    for i in word_lists:
        data_socket.send(pacman_msg_format.format_msg('DATA',i))
        #print(i)
        time.sleep(1)
    data_socket.close()
    ctx.destroy()


word_lists = hdf5ToPackets(data_file)
print("Starting PACMAN card(s)")
socket = sender_setup(data_socket)
import multiprocessing
process = multiprocessing.Process(target=sender,args=[socket,word_lists])
#process = multiprocessing.Process(target=sender,args=[1000])
process.daemon = True
process.start()
