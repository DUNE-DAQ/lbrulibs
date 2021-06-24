import zmq
import time
import multiprocessing
import larpix
from larpix.format import pacman_msg_format

cmd = 'tcp://127.0.0.1:5555'
data = 'tcp://127.0.0.1:5556'
N_READOUTS = 1 #number of readouts
datafile = "readout-test.h5"

def readout():
    try:
        commander = zmq.Context().socket(zmq.REQ)
        commander.connect(cmd)
    
        # Using SUB socket to collect data
        reader = zmq.Context().socket(zmq.SUB)
        reader.connect(data)
        reader.setsockopt(zmq.SUBSCRIBE, b"")
        
        print("Press ENTER to start listening...")
        input()
        '''
        commander.send(b'')
        print("Signal sent to PACMAN card.")
        commander.recv()
        '''

        messages = 0
        while True:
            print("Listening on port:",data,"...")
            message = reader.recv()
            print("Message received:", message)
            messages += 1
            print("Total messages received:", messages)
   
            print("Converting to a packet...")
            packet = pacman_msg_format.parse(message)
            print("Writing to HDF5 file:", datafile)
            larpix.format.hdf5format.to_file(datafile,packet)
            print("Message written to file.")
            
            
    except:
        raise
    finally:
        reader.close()


def main():
    # Start tasks
    def start(task, *args):
        process = multiprocessing.Process(target=task, args=args)
        process.daemon = True
        process.start()
    for i in range(N_READOUTS):
        start(readout(), i)

if __name__ == "__main__":
    main()