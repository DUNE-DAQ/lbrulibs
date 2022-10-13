#!/usr/bin/env python3

from hdf5libs import HDF5RawDataFile

import daqdataformats
import detdataformats

import click
import time
import numpy as np


@click.command()
@click.argument('filename', type=click.Path(exists=True))

def main(filename):

    h5_file = HDF5RawDataFile(filename)

    records_to_process = h5_file.get_all_record_ids()
    print(f'Will process {len(records_to_process)}')

    for r in records_to_process:

        print(f'Processing (Record Number,Sequence Number)=({r[0],r[1]})')
        mpd_geo_ids = h5_file.get_geo_ids_for_subdetector(r,detdataformats.DetID.Subdetector.kNDLAr_PDS)

        for gid in mpd_geo_ids:
            hex_gid = format(gid, '016x')
            print(f'\tProcessing geoid {hex_gid}')

            frag = h5_file.get_frag(r,gid)
            frag_hdr = frag.get_header()
            frag_ts = frag.get_trigger_timestamp()

            print(f'\tTrigger timestamp for fragment is {frag_ts}')

            mpd_f = detdataformats.mpd.MPDFrame(frag.get_data())

            #print header info
            print('\n\t==== MPD HEADER (First Frame) ====')
            header = mpd_f.get_header()
            
            #Check if Timestamp Sync number is correct
            if str(hex(header.timestamp_sync)) != '0x3f60b8a8' : 
                print ("\t\t \033[1m\033[91m*** EMPTY FRAGMENT ***\033[0m\033[0m")
            prefix = '\t\t'
            print(f'{prefix} Timestamp Sync: {hex(header.timestamp_sync)}')
            print(f'{prefix} Timestamp size: {header.timestamp_length}')
            print(f'{prefix} TimeStamp OS: {header.timestamp_OS}')
            print(f'{prefix} SyncMagic: {hex(header.SyncMagic)}')
            print(f'{prefix} Length: {header.length}')
            print(f'{prefix} \033[1mEvent number: {header.event_num} \033[0m ')
            print(f'{prefix} Device serial number: {header.device_serial_num}')
            print(f'{prefix} Device length: {header.device_length}')
            print(f'{prefix} Device model ID: {header.device_model_id}')
            print(f'{prefix} Trigger type: {header.trigger_type}')
            print(f'{prefix} Trigger length: {header.trigger_length}')
            print(f'{prefix} Trigger channel number: {header.trigger_channel_number}')
            print(f'{prefix} Event timestamp 1: {header.event_timestamp_1}')
            print(f'{prefix} Event timestamp 2: {header.event_timestamp_2}')
            print(f'{prefix} Flags: {header.flags}')
            print(f'{prefix} Channel bit mask: {header.channel_bit_mask}')
            print(f'{prefix} Data type: {header.data_type}')
            print(f'{prefix} Data length: {header.data_length}')
            print(f'{prefix} Channel number: {header.channel_number}')

            print("\n")
        #end gid loop
 
    print(f'Processed all requested records')

if __name__ == '__main__':
    main()
