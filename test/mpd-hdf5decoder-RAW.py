#!/usr/bin/env python3

from hdf5libs import HDF5RawDataFile

import daqdataformats
import detdataformats

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
            OSheader = mpd_f.get_OSheader()
            event_header = mpd_f.get_event_header()
            device_header = mpd_f.get_device_header()
            trigger_header = mpd_f.get_trigger_header()
            trigger_data_header = mpd_f.get_trigger_data_header()
            data_header = mpd_f.get_data_header()
            
            #Check if Timestamp Sync number is correct
            prefix = '\t\t'
            print(f'{prefix} Timestamp Sync: {hex(OSheader.timestamp_sync)}')
            print(f'{prefix} Timestamp size: {OSheader.timestamp_length}')
            print(f'{prefix} TimeStamp OS: {OSheader.timestamp_OS}')
            print(f'{prefix} SyncMagic: {hex(event_header.SyncMagic)}')
            print(f'{prefix} Length: {event_header.length}')
            print(f'{prefix} \033[1mEvent number: {event_header.event_num} \033[0m ')
            print(f'{prefix} Device serial number: {device_header.device_serial_num}')
            print(f'{prefix} Device length: {device_header.device_length}')
            print(f'{prefix} Device model ID: {device_header.device_model_id}')
            print(f'{prefix} Trigger type: {trigger_header.trigger_type}')
            print(f'{prefix} Trigger length: {trigger_header.trigger_length}')
            print(f'{prefix} Trigger channel number: {trigger_header.trigger_channel_number}')
            print(f'{prefix} Event timestamp 1: {trigger_data_header.event_timestamp_1}')
            print(f'{prefix} Event timestamp 2: {trigger_data_header.event_timestamp_2}')
            print(f'{prefix} Flags: {trigger_data_header.flags}')
            print(f'{prefix} Channel bit mask: {trigger_data_header.channel_bit_mask}')
            print(f'{prefix} Data type: {data_header.data_type}')
            print(f'{prefix} Data length: {data_header.data_length}')
            print(f'{prefix} Channel number: {data_header.channel_number}')
            print(f'{prefix} \033[1mEpoch Time Stamp: {mpd_f.get_timestamp()}\033[0m')
            if str(hex(OSheader.timestamp_sync)) != '0x3f60b8a8' : 
                print ("\t\t \033[1m\033[91m*** EMPTY FRAGMENT ***\033[0m\033[0m")
                count_invalid += 1
            else :
                if prev_timestamp != 0 : 
                    print(f'{prefix} Timestamp({event_header.event_num}) - Timestamp({event_header.event_num -1 }) = {mpd_f.get_timestamp() - prev_timestamp} ')
                prev_timestamp = mpd_f.get_timestamp()
            print("\n")
        #end gid loop
    
    print(f'Processed all requested records')
    print(f'Valid processed: {len(records_to_process)-count_invalid}')
    print(f'Invalid processed: {count_invalid}')

if __name__ == '__main__':
    main()
