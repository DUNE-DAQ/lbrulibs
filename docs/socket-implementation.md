# Socket implementation in lbrulibs

As of 2.11 the plugin by default uses ZMQ "STREAM" sockets, which can receive messages from any TCP source. The use ZMQ "subscriber" socket is planned to become deprecated by daq framework 3.X

## Sending data to lbrulibs readout (for frontend developers)
In order to properly connect to lbrulibs:
   1. Front ends "connect", DAQ "binds"
   2. Connection opened by sending an empty frame (standard TCP, see your library/implementation documentation, in ZMQ handled for you by connect call)
   3. If connection attempt fails on front end side they need to retry every X amount of time (avoid immediately retrying to not send too many of connection requests saturating the TCP buffer, 1 second+ before retries seems reasonable)
   4. Upon connection front ends receive a 2 part message: routing id + empty frame, in ZMQ this can be received with 2 receive calls or a receive_multipart, in other implementations see their documentation/general TCP standard rules
   5. Front ends will send to that routing id, standard TCP setup of id+data, in ZMQ send multipart(tuple of id an data) or send two messages for id and data, in other implementations see their documentation/general TCP standard rules