#general imports
import time
import sys
import multiprocessing
#import h5py
import random

#larpix imports
import larpix
from larpix.format import pacman_msg_format

#zmq imports
import zmq

# Prepare ports
echo = 'tcp://127.0.0.1:5554'
cmd = 'tcp://127.0.0.1:5555'
data = 'tcp://127.0.0.1:5556'

N_PACMAN = 1 #number of PACMAN cards
N_REPEATS = 2 #number of repetitions of the data

# Converts HDF5 files into a list of PACMAN messegaes (bytes)
def hdf5ToPackets(datafile): 
    print("Reading from:",datafile)
    packets = larpix.format.hdf5format.from_file(datafile)['packets'] #read from HDF5 file
    print("Separating into messages based on timestamp packets...")
    msg_breaks = [i for i in range(len(packets)) if packets[i].packet_type == 4 or i == len(packets)-1] #find the timestamp packets which signify message breaks
    msg_packets = [packets[i:j] for i,j in zip(msg_breaks[:-1], msg_breaks[1:])] #separate into messages
    msgs = [pacman_msg_format.format(p, msg_type='DATA') for p in msg_packets]
    print("Extracting headers and words from messages...")
    #header_list = [pacman_msg_format.parse_msg(p)[0] for p in msgs] #retrieve headers
    word_lists = [pacman_msg_format.parse_msg(p)[1] for p in msgs] #retrieve lists of words from each message
    
    '''
    # Deprecated method of making messages
    messages = []
    print("Creating a message every second...")
    for i in msg_packets:
        messages.append(pacman_msg_format.format(i, msg_type='DATA'))
        print(pacman_msg_format.parse(pacman_msg_format.format(i, msg_type='DATA')))
        time.sleep(1)
    '''

    '''
    # Modern method of making messages
    messages = []
    print("Creating a message every second...")
    for i in word_lists:
        messages.append(pacman_msg_format.format_msg('DATA',i))
        print(pacman_msg_format.parse_msg(pacman_msg_format.format_msg('DATA',i)))
        time.sleep(1)
    print("Messages created.")
    '''

    print("Read complete. PACMAN style messages prepared.")
    return word_lists

# Instance of a PACMAN card
def pacman(_echo_server,_cmd_server,_data_server,word_lists,nRepeats=1):
    try:
        # Set up sockets
        print("Setting up ZMQ sockets...")
        ctx = zmq.Context()
        cmd_socket = ctx.socket(zmq.REP)
        data_socket = ctx.socket(zmq.PUB)
        echo_socket = ctx.socket(zmq.PUB)
        socket_opts = [
            (zmq.LINGER,100),
            (zmq.RCVTIMEO,100),
            (zmq.SNDTIMEO,100)
        ]
        print("Parsing socket options...")
        for opt in socket_opts:
            cmd_socket.setsockopt(*opt)
            data_socket.setsockopt(*opt)
            echo_socket.setsockopt(*opt)
        print("Binding sockets...")
        cmd_socket.bind(_cmd_server)
        data_socket.bind(_data_server)
        echo_socket.bind(_echo_server)
        
        '''
        # Synchronisation with readout
        # Set up a poller, wait for signal from readout to start sending data
        print("Setting up a poller for registering CCM commands...")
        print("Waiting for signal from readout to start sending data...")
        poller = zmq.Poller()
        poller.register(cmd_socket, zmq.POLLIN)
        items = dict(poller.poll())
        if cmd_socket in items:
            message = cmd_socket.recv()
            print("Signal received.")
            cmd_socket.send(b'')
        '''
        
        print('Press any key to start sending data...')
        input()
        print('Initialising...')
        time.sleep(1)
        print("Data will repeat %i times." %nRepeats)
        print('Sending PACMAN messages.')

        # Send messages in intervals based on timestamps
        message_count = 0
        
        for n in range(nRepeats):
            for i in word_lists:
                data_socket.send(pacman_msg_format.format_msg('DATA',i))
                print(pacman_msg_format.parse_msg(pacman_msg_format.format_msg('DATA',i)))
                message_count += 1
                print("Total messages sent:",message_count)
                next_sleep = random.randrange(1,3)
                if message_count != len(word_lists)*nRepeats:
                    print("Next message in: %ds" %(next_sleep))
                    time.sleep(next_sleep)
            
    except:
        raise
    finally: #cleanup
        data_socket.close()
        cmd_socket.close()
        echo_socket.close()
        ctx.destroy()


def main(*args):
    # Fetch messages and timestamps
    word_lists = hdf5ToPackets(args[0])
    print("Starting PACMAN card(s)")
    start_time = time.time()
    # Start PACMAN cards
    def start(task, *args):
        process = multiprocessing.Process(target=task, args=args)
        process.daemon = True
        process.start()
    for i in range(N_PACMAN):
        start(pacman(echo,cmd,data,word_lists,N_REPEATS), i)
    print("Total elapsed time:",time.time()-start_time)


if __name__ == "__main__":
    try:
        main(str(sys.argv[1]))
    except IndexError:
        print("ERROR - Need to provide a data file!")
