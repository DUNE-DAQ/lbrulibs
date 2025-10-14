import larpixtools
from rich.progress import Progress, SpinnerColumn, BarColumn, TextColumn


# Converts HDF5 files into a list of PACMAN messegaes (bytes)
def hdf5ToPackets(datafile): 
    print("Reading from:",datafile)
    packets = larpixtools.from_file(datafile)['packets'] #read from HDF5 file
    print("Separating into messages based on timestamp packets...")
    msg_breaks = [i for i in range(len(packets)) if packets[i].packet_type == 0 or i == len(packets)-1] #find the timestamp packets which signify message breaks
    msg_packets = [packets[i:j] for i,j in zip(msg_breaks[:-1], msg_breaks[1:])] #separate into messages
    msgs = [larpixtools.format(p, msg_type='DATA') for p in msg_packets]
    print("Extracting headers and words from messages...")
    #header_list = [pacman_msg_format.parse_msg(p)[0] for p in msgs] #retrieve headers
    word_lists = [larpixtools.parse_msg(p)[1] for p in msgs] #retrieve lists of words from each message    
    print("Read complete. PACMAN style messages prepared.")
    return word_lists


def convert_bytes_to_file(word_list, output_file):
    with open(output_file, "wb") as f:
        # add rich loading bar
        with Progress(
            SpinnerColumn(),
            TextColumn("[bold blue]{task.fields[filename]}"),
            BarColumn(),
            transient=True,
        ) as progress:
            task = progress.add_task("[green]Converting to bytes...", filename=output_file)
            for word in word_list:
                f.write(larpixtools.format_msg('DATA', word))
                progress.update(task, advance=1)
    
if __name__=="__main__":
    # Add input file comand line option
    import argparse
    parser = argparse.ArgumentParser(description='Convert HDF5 to PACMAN messages')
    parser.add_argument('--input_file', type=str, help='Input HDF5 file')
    # Compile parser
    args = parser.parse_args()
    
    word_lists = hdf5ToPackets(args.input_file)
    
    output_file = args.input_file.replace(".h5", ".bin")
    convert_bytes_to_file(word_lists, output_file)