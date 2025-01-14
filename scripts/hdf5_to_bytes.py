import h5py

def hdf5ToPackets(datafile: str, output_file: str): 
    print(f"Reading from data file {datafile}")
    
    f =  h5py.File(datafile, 'r')
    f_packets = f['packets']
    
    
    with open(output_file, 'wb') as o:
        for p in f_packets:
            o.write(p)
    
    f.close()
    
if __name__=="__main__":
    INPUT_FILE = "test/example-pacman-data.h5"
    OUTPUT_FILE= "test/example-pacman-data.bin"
    hdf5ToPackets(INPUT_FILE, OUTPUT_FILE)