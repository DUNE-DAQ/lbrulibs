import zmq
import time
import multiprocessing
#import larpix
#from larpix.format import pacman_msg_format
#sys.path.insert(1, '../scripts')
import larpixtools

data = 'tcp://127.0.0.1:5556'
N_READOUTS = 1 #number of readouts
datafile = "readout-test-RAW.h5"

def readout():
    try:
        # Using STREAM socket to collect data
        print("Initialising...")
        reader = zmq.Context().socket(zmq.STREAM)
        #reader.bind(data)
        reader.bind(data)
        time.sleep(1)
        print("Press ENTER to start listening...")
        input()
        print("Listening...")

        messages = 0
        while True:
            print("Listening on port:",data,"...")
            # need to receive two messages, or recv_multipart()
            id = reader.recv()
            message = reader.recv()
            print("Message received:", message, "from:",id)
            if message != b"":
                messages += 1
                print("Total messages received:", messages)
   
                print("Converting to a packet...")
                packet = larpixtools.parse(message)
                print(packet)
                #print("Writing to HDF5 file:", datafile)
                #larpix.format.hdf5format.to_file(datafile,packet)
                #print("Message written to file.")
            
            
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
