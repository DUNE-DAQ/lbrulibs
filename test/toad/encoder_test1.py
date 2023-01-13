from encoder import encode_hdr, encode_data, encoder
import time
from random import gauss, uniform
import numpy as np
import sys

timestamp = int(time.time())

for i in range(int(sys.argv[1])):
    timestamp += 1
    hdr_par = 1
    data_par = 0
    res = 0
    idd = 1

    clustercount = 0

    file = "testdata_1hdr_" + str(i) + ".txt"
    file2 = "testdata_bytes_1hdr_"+str(i)+".txt"
    file3 = "testdata_32bit_1hdr_"+str(i)+".txt"

    f = open(file, "w")
    x = open(file2, "w")
    h = open(file3, "w")

    fec_num = int(uniform(0, 511))
    channel_id = int(uniform(0, 31))
    num_clusters = np.random.poisson(5)
    print(clustercount)
    padd = 6 - (num_clusters % 6)
    encoder(file, file2, file3, fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
    f.close()
    x.close()
    h.close()
    time.sleep(0.5)

'''
timestamp = int(time.time())
hdr_par = 1
data_par = 0
#padd = num_clusters % 6
res = 0
idd = 1

clustercount = 0

file = "testdata_idtest.txt"
file2 = "testdata_bytes_idtest.txt"
file3 = "testdata_32bit_idtest.txt"

f = open(file, "w")
x = open(file2, "w")
h = open(file3, "w")
count = 0
#for 4k bytes, set range to 235 and count>4072, count<4096
for i in range(525):
    fec_num = int(uniform(0, 511)) 
    channel_id = int(uniform(0, 31))
'''
'''
    if(count>4072 and count <4096):
        z = 4096 - count - 8
        print(z)
        timestamp = timestamp + 1
        num_clusters = int((z-8)*6/8)
        padd = 6 - (num_clusters % 6)
        encoder(file, file2,file3,fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
        break
'''
'''
    num_clusters = np.random.poisson(5)
    clustercount += num_clusters
    print(clustercount)
    timestamp = timestamp + 1
    padd = 6 - (num_clusters % 6)
    encoder(file, file2, file3, fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
    count = count + 8 + 8*int((num_clusters+5)/6)
'''
'''
for i in range(0, 64, 1):
    y = gauss(5, 5)
    if(y>0):
        num_clusters = int(y)
    if(y<=0):
        continue
    timestamp = timestamp + 1
    padd = num_clusters % 6
    encoder(file, file2,file3, fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
    count = count + 8 + 8*int((num_clusters+5)/6)
    if(count>1000 and count <1024):
        z = 1024 - count
        print(z)
        timestamp = timestamp + 1
        num_clusters = int((z-8)*6/8)
        encoder(file, file2,file3,fec_num, num_clusters, channel_id, timestamp, hdr_par, data_par, padd, res, idd)
        break
'''
'''
f.close()
x.close()
h.close()
'''
