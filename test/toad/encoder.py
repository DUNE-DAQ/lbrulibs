#import ROOT
import numpy as np
import time
from random import uniform

def encode_hdr(filename, filename_bytes, filename_bytes_2, fec_num_in, num_clusters_in, channel_id_in, timestamp_in, hdr_par_in, data_par_in, padd_in, res_in, idd_in):
    f = open(filename, "a")
    x = open(filename_bytes, "a")
    h = open(filename_bytes_2, "a")
    fec_num = fec_num_in
    f.write("%d\n" %fec_num)
    num_clusters = num_clusters_in
    f.write("%d\n" %num_clusters)
    channel_id = channel_id_in
    f.write("%d\n" %channel_id)
    timestamp = timestamp_in
    f.write("%d\n" %timestamp)
    hdr_par = hdr_par_in
    f.write("%d\n" %hdr_par)
    data_par = data_par_in
    f.write("%d\n" %data_par)
    padd = padd_in
    f.write("%d\n" %padd)
    res = res_in
    f.write("%d\n" %res)
    idd = idd_in
    f.write("%d\n" %idd)
    word = idd
    word = (word<<2) | res
    word = (word<<3) | padd
    word = (word<<1) | data_par
    word = (word<<1) | hdr_par
    word = (word<<32) | timestamp
    word = (word<<5) | channel_id
    word = (word<<10) | num_clusters
    word = (word<<9) | fec_num

    #print(word)
    word_final = int(bin(word), 2)
    #print(word_final)
    count = 0
    for i in range(0, 64, 8):
        byte_num = (word_final>>i) & 0xFF
        x.write("%d\n" %byte_num)
    #print(bin(word_final))
    for i in range(0, 64, 32):
        four_byte_num = (word_final>>i) & 0xFFFFFFFF
        h.write("%d\n" %four_byte_num)
    f.close()
    h.close()
    x.close()
    return None

def encode_data(filename, filename_bytes, filename_bytes_2, num_clusters_in, padd_in):
    f = open(filename, "a")
    x = open(filename_bytes, "a")
    h = open(filename_bytes_2, "a")
    idd = 0
    f.write("%d\n" %idd)
    word = idd
    padd = padd_in
    f.write("%d\n" %padd)
    word = (word<<3) | padd
    if(num_clusters_in<6):
        for i in range(0, 6-num_clusters_in, 1):
            padded_data = 0
            word = (word<<10) | padded_data
            f.write("%d\n" %padded_data)
    for i in range(num_clusters_in):
        data_cluster = int(uniform(1, 1023))
        #print(data_cluster)
        word = (word<<10) | data_cluster
        f.write("%d\n" %data_cluster)
    word_final = int(bin(word), 2)
    #print(word_final)
    for i in range(0, 64, 8):
        byte_num = (word_final>>i) & 0xFF
        x.write("%d\n" %byte_num)
    for i in range(0, 64, 32):
        four_byte_num = (word_final>>i) & 0xFFFFFFFF
        h.write("%d\n" %four_byte_num)
    f.close()
    x.close()
    h.close()
    return None

def encoder(filename, filename_bytes,filename_bytes_2, fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd):
    encode_hdr(filename, filename_bytes, filename_bytes_2, fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
    for i in range(0, num_clusters, 6):
        if((num_clusters-i)>=6):
            encode_data(filename, filename_bytes,filename_bytes_2,6, 0)
        elif((num_clusters-i)<6):
            x = num_clusters - i
            y = 6 - x
            #print(x)
            encode_data(filename, filename_bytes, filename_bytes_2, x, y)
    return None
'''
fec_num = 10
num_clusters = 5
channel_id = 1
timestamp = int(time.time())
hdr_par = 1
data_par = 0
padd = 1
res = 3
idd = 1

file = "testdata.txt"
file2 = "testdata_bytes.txt"

f = open(file, "w")
x = open(file2, "w")
encoder(file, file2,fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
f.close()
x.close()
'''
