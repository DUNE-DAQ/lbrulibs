import subprocess
import time
import sys

for i in range(int(sys.argv[1])): #argument in command line is number of txt files to send
    arg = "testdata_bytes_1hdr_"+str(i)+".txt"
    subprocess.call(["./encoded_client_test.out", arg])
    time.sleep(1)
    print("one loop finished")


