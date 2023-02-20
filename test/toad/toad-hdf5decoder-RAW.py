#!/usr/bin/env python3
from hdf5libs import HDF5RawDataFile

import daqdataformats
import detdataformats
import sys
import click
import time
import numpy as np
import datetime

@click.command()
@click.argument('filename', type=click.Path(exists=True))

def main(filename):

    h5_file = HDF5RawDataFile(filename)

    records_to_process = h5_file.get_all_record_ids()
    print(f'Will process {len(records_to_process)}')

    count_invalid = 0 
    prev_timestamp = 0 
    for r in records_to_process:

        print(f'Processing (Record Number,Sequence Number)=({r[0],r[1]})')
        toad_geo_ids = h5_file.get_geo_ids_for_subdetector(r,detdataformats.DetID.Subdetector.kND_GAr)

        for gid in toad_geo_ids:
            hex_gid = format(gid, '016x')
            print(f'\tProcessing geoid {hex_gid}')

            frag = h5_file.get_frag(r,gid)
            frag_hdr = frag.get_header()
            frag_ts = frag.get_trigger_timestamp()

            print(f'\tTrigger timestamp for fragment is {frag_ts}')
            toad_f = detdataformats.toad.TOADFrameOverlay(frag.get_data())

            #print header info
            print('\n\t==== TOAD FRAGMENT ====') 

            #Check if Timestamp Sync number is correct
            prefix = '\t\t'
            print(f'{prefix} Timestamp in ticks: {toad_f.tstmp}')
            print(f'{prefix} FEC number: {toad_f.fec}')
            print(f'{prefix} Header Parity Check: {toad_f.hdr_par_check}') 
            print(f'{prefix} Number of samples: {toad_f.n_samples}') 
            
    print(f'Processed all requested records')
    print(f'Valid processed: {len(records_to_process)-count_invalid}')
    print(f'Invalid processed: {count_invalid}')

if __name__ == '__main__':
    main()
