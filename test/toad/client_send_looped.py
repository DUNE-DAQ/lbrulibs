import subprocess
import time
import sys

###
#for i in range(int(sys.argv[1])): #argument in command line is number of txt files to send
#    arg = "testdata_newformat_"+str(i)+".txt"
#    subprocess.call(["./encoded_client_test.out", arg])
#    time.sleep(0.5)
#    print("one loop finished")
###

while True:
    arg = "testdata_newformat.txt"
    subprocess.call(["./encoded_client_test.out", arg])
    time.sleep(2)
    print("one loop finished")
