import os
import numpy as np
import argparse
import sys
import struct
from collections import deque
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

        print('Press any key to start sending data...')
        input()
        print('Initialising...')
        time.sleep(1)
        print('Sending ', n_packets, ' MPD messages.')

        # Send messages in intervals based on timestamps
        message_count = 0
        for i in range(n_packets):
            data_socket.send_multipart([id,packets[i]])
            
            message_count += 1
            print("Total messages sent:",message_count)        

            next_sleep = random.randrange(1,3)
            if message_count != n_packets :
                print("Next message in: %ds" %(next_sleep))
                time.sleep(next_sleep);
                break;

        print("Sleeping for 10 seconds before exiting...")
        time.sleep(10)
    except:
        raise
    finally: #cleanup
        data_socket.close()
        ctx.destroy()

if __name__ == "__main__":
    parser = argparse.ArgumentParser();
    # Add arguments here for my script
    parser.add_argument('--input-file', '-i', dest='input_file', type=str, default="example-mpd-data.data", help="Input file which contains data to be sent to stream.")
    parser.add_argument('--num-packets', '-n', dest='num_packets', type=int, default=40050, help="Number of packets to send to socket. Default is the number of packets stored in mps example file")
    args = parser.parse_args();

    print('Using',args.input_file,'. Decoding its members ...')

    mpd_data = mpd.mpd(args.input_file, args.num_packets)

    send_mpd(mpd_data.packets, mpd_data.num_packets())
