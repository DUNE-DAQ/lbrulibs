import os
import numpy as np
import argparse
import sys
import struct
from collections import deque

class mpd :
    def __init__( self, fileName, n_file_evals, num_packets ) : 
        self.packets = deque()
        self.HEADER_SIZE = 28 ; # bytes

        file_bytes = os.path.getsize(fileName)
        with open(fileName, mode='rb') as file: 
            fileContent = file.read()

            for i in range(n_file_evals):
                fb_i = 0 # first byte in packet
                fb_f = 0 # last byte in packet
                while fb_i < file_bytes :
                    fb_f += self.HEADER_SIZE + struct.unpack('i', fileContent[fb_i+20:fb_i+24])[0] 
                    self.packets.append( fileContent[fb_i:fb_f] ) # store packet information in binary
                    fb_i = fb_f 

                    if len(self.packets) == num_packets :
                        break
                
    def print_packet_info( self, event ):
        # This is a helper function that translates the bytes into readable information
        # Only for testing purposes 

        fileContent = self.packets[event]
        # Timestamp Header
        timestamp_sync = struct.unpack('i', fileContent[:4])[0] # Should be 0x3f60b8a8
        timestamp_length = struct.unpack('i', fileContent[4:8])[0] # Should be 0x00000008 (int64 bits)
        timestamp = struct.unpack('q', fileContent[8:16])[0]
        
        #Event Header
        event_sync_numb = struct.unpack('i', fileContent[16:20])[0] # 0x2A502A50
        tot_length = struct.unpack('i', fileContent[20:24])[0] 
        # total length in bytes of all device event blocks including their headers
        event_number = struct.unpack('i', fileContent[24:28])[0]
        
        print('Time stamp sync = ',hex(timestamp_sync))
        print('Timestamp length = ',hex(timestamp_length))
        print('Time stamp operation system = ',timestamp)
        print('Event sync number =',hex(event_sync_numb))
        print('Total length event header =',tot_length)
        print('Event Number =',event_number)
        
    def num_packets(self):
        return len(self.packets)

    def time_stamp_event(self, event):
        fileContent = self.packets[event]
        return struct.unpack('q', fileContent[8:16])[0]
