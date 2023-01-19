#general imports
import argparse
import time
import sys
import random
import numpy as np

#zmq imports
import zmq

#prepare ports
data = 'tcp://127.0.0.1:5556'

# Read bytes from txt file
def ReadTxt(datafile):
  print("Reading from:",datafile)
  fileobj = open(datafile, "r")
  bytes_list = np.array([])
  for line in fileobj:
    np.append(bytes_list, line)
  print("Completed File Reading")
  return bytes_list

#no modes, will send the entire list in one function and then will be split into TOAD Packets during unpacking

# Instance of a TOAD card
def toad(_data_server,word_lists,n_file_evals=1):
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
        data_socket.connect(_data_server)
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
    print("Data will repeat %i times." %(n_file_evals-1))
    print('Sending Bytes')
    
    message_count = 0
    for n in range(n_file_evals):
      data_socket.send_multipart([id, word_lists])
      message_count += 1
      print("Total messages sent:",message_count)
      next_sleep = random.randrange(1,3)
      if message_count != n_file_evals:
        print("Next message in: %ds" %(next_sleep))
        time.sleep(next_sleep)

    print("Sleeping for 10 seconds before exiting...")
    time.sleep(10)
  finally: #cleanup
    data_socket.close()
    ctx.destroy()

def main(s_in_file, n_file_evals):
    if(s_in_file == "ns"):
        print("\n\n\nPlease specify an input file...")
        sys.exit()

    # Fetch messages and timestamps
    word_lists = ReadTxt(s_in_file)
    toad(data,word_lists,n_file_evals)

if __name__ == "__main__":
    parser = argparse.ArgumentParser();
    parser.add_argument('--input_file', '-i', dest='input_file',   type=str, default="ns",  help='Input h5 file.')
    parser.add_argument('--n_file_evals',     dest='n_file_evals', type=int, default=1,     help='Number of times the input file is looped through.')
    args = parser.parse_args();

    main(args.input_file, args.n_file_evals);

