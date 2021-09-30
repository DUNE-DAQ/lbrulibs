#general imports
import argparse
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

# Useful print message for the user which tells them what they are about to run.
def print_explain_modes():
    print("""
          Running mode 0:
          You will send all of the messages in the input file individually at
          intervals of 1 to 3 seconds drawn randomly. If you specified --n_messages_total,
          --n_messages_group or --group_interval, this will be ignored.\n
          Running mode 1:
          For each loop of the input file you specified, you will send groups of messages
          of size, split by intervals of and totalling the numbers you specify using
          --n_messages_total, --n_messages_group and --group_interval.\n
          Running mode 2:
          For each loop of the input file you will send a single message.
          If you specified --n_messages_total, --n_messages_group or --group_interval, this will be ignored.\n
          Running mode 3:
          For each loop of the input file you will send 10 total messages individually
          spaced by 1 second.
          If you specified --n_messages_total, --n_messages_group or --group_interval, this will be ignored.\n
          Running mode 4:
          For each loop of the input file you will send 50 total messages in groups of 5
          spaced by 1 second.
          If you specified --n_messages_total, --n_messages_group or --group_interval, this will be ignored.
          """)
    sys.exit()
    return

def print_mode_info (mode, this_n_messages_total, this_n_messages_group, this_group_interval):
    
    s_print = "\n\n\n Running mode " + str(mode) + "\n" 
    if mode == 0:
        s_print += """ You will send all of the messages in the input file individually at
        intervals of 1 to 3 seconds drawn randomly. If you specified --n_messages_total,
        --n_messages_group or --group_interval, this will be ignored.
                   \n\n\n"""
    elif mode == 1:
        s_print += " For each loop of the input file you specified, you will send " + str(this_n_messages_total) + " total"
        s_print += " messages in groups of " + str(this_n_messages_group)
        s_print += " at intervals of " + str(this_group_interval) + " seconds.\n\n\n"
    elif mode == 2:
        s_print += """ For each loop of the input file you will send a single message.
        If you specified --n_messages_total, --n_messages_group or --group_interval, this will be ignored.
                   \n\n\n"""
    elif mode == 3:
        s_print += """ For each loop of the input file you will send 10 total messages individually
        spaced by 1 second.
        If you specified --n_messages_total, --n_messages_group or --group_interval, this will be ignored.
                   \n\n\n"""
    elif mode == 4:
        s_print += """ For each loop of the input file you will send 50 total messages in groups of 5
                   spaced by 1 second.
                   If you specified --n_messages_total, --n_messages_group or --group_interval, this will be ignored.
                   \n\n\n"""

    print(s_print)
    return


# Instance of a PACMAN card
def pacman(_echo_server,_cmd_server,_data_server,word_lists,mode,n_messages_total,n_messages_group,group_interval,n_file_evals=1):
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
        #print("Data will repeat %i times." %(n_file_evals-1))
        print("Data will repeat %i times." %(n_file_evals-1))
        print('Sending PACMAN messages.')

        # MODE 0: Default (and original) behaviour. Single messages sent at randon intervals between 1 and 3 seconds.
        # MODE 1: User specifies one or all of n_messages_total, n_messages_group and group_interval.
        #         If a particular one isn't specified, it is just set to its default value. 
        # MODES 2 AND ABOVE: Predefined n_messages_total, n_messages_group and group_interval. If user specifies these parameters
        #                    they will be ignored.
        # MODE 2: Single message.
        # MODE 3: 10 total message sent individually at intervals of 1 second.
        # MODE 4: 50 total message sent in groups of 5 at intervals of 1 second.
        if mode < 3:
            this_n_messages_total = n_messages_total
            this_n_messages_group = n_messages_group
            this_group_interval   = group_interval
        if mode == 3:
            this_n_messages_total = 10
            this_n_messages_group = 1
            this_group_interval   = 1
        if mode == 4:
            this_n_messages_total = 50
            this_n_messages_group = 5
            this_group_interval   = 1

        print_mode_info (mode, this_n_messages_total, this_n_messages_group, this_group_interval);

        # Send messages in intervals based on timestamps
        message_count = 0
        
        for n in range(n_file_evals):
            for i in word_lists:
                #data_socket.send(b"", zmq.SNDMORE)
                data_socket.send(pacman_msg_format.format_msg('DATA',i))
                print(pacman_msg_format.parse_msg(pacman_msg_format.format_msg('DATA',i)))
                message_count += 1
                print("Total messages sent:",message_count)
                if mode == 2: break;
                elif mode > 0:
                    if message_count % this_n_messages_total == 0: 
                        time.sleep(next_sleep);
                        break;
                    if message_count % this_n_messages_group == 0: 
                        next_sleep = this_group_interval;
                    else: continue;
                else:
                    next_sleep = random.randrange(1,3)
                    if message_count != len(word_lists)*n_file_evals:
                        print("Next message in: %ds" %(next_sleep))

                time.sleep(next_sleep)
            
        print("Sleeping for 10 seconds before exiting...")
        time.sleep(10)
    except:
        raise
    finally: #cleanup
        data_socket.close()
        cmd_socket.close()
        echo_socket.close()
        ctx.destroy()


def main(s_in_file, mode, n_file_evals, n_pacman, n_messages_total, n_messages_group, group_interval, explain_modes):

    if(explain_modes): print_explain_modes()
    if(s_in_file == "ns"):
        print("\n\n\nPlease specify an input file...")
        sys.exit()

    # Fetch messages and timestamps
    word_lists = hdf5ToPackets(s_in_file)
    print("Starting PACMAN card(s)")
    start_time = time.time()
    # Start PACMAN cards
    def start(task, *args):
        process = multiprocessing.Process(target=task, args=args)
        process.daemon = True
        process.start()
    for i in range(n_pacman):
        start(pacman(echo,cmd,data,word_lists,mode,n_messages_total,n_messages_group,group_interval,n_file_evals), i)
    print("Total elapsed time:",time.time()-start_time)


if __name__ == "__main__":

    parser = argparse.ArgumentParser();
    parser.add_argument('--explain_modes', dest='explain_modes', action='store_true', help="Print an explanation of the running modes.")
    parser.add_argument('--input_file',    dest='input_file',   type=str, default="ns",  help='Input h5 file.')
    parser.add_argument('--mode',          dest='mode',         type=int, default=0,     help='Running mode, can take values [0-4]. Pass --explain_modes to this script for a full explanation of different modes.')
    parser.add_argument('--n_file_evals',  dest='n_file_evals', type=int, default=1,     help='Number of times the input file is looped through.')
    parser.add_argument('--n_pacman',      dest='n_pacman',     type=int, default=1,     help='Number of PACMAN cards.')
    parser.add_argument('--n_messages_total', dest='n_messages_total', type=int,   default=10,  help='To be used with --mode 1. Total number of messages sent during one loop of input file.')
    parser.add_argument('--n_messages_group', dest='n_messages_group', type=int,   default=1,   help='To be used with --mode 1. Total number of messages sent at once at intervals of --group_interval.')
    parser.add_argument('--group_interval',   dest='group_interval',   type=float, default=1.0, help='To be used with --mode 1. Time interval between groups of messages being sent.')
    args = parser.parse_args();

    main(args.input_file, args.mode, args.n_file_evals, args.n_pacman, args.n_messages_total, args.n_messages_group, args.group_interval, args.explain_modes);

