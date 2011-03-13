Cmpt471 Spring 2011 SFU Surrey

Author Cal Woodruff <cwoodruf@sfu.ca> 301013983

Design:

The arp cache is checked every 10 seconds and is refreshed every 60 seconds.
Arp data is held in an array. The IP address of the destination is hashed
to find data. Hash collisions are virtually non-existent given the 
way the subnet is set up with all IP addresses sharing the same 29 bit CIDR 
prefix. Nevertheless the algorithm for handling hash collisions is to 
count through the arp array until an available array index is found.
Arp code is found in sr_arp.c/h.

Packet processing is implemented in sr_ip.c/h. A single c structure overlay 
was used to access headers and data in the packets. 

The routing of packets is implemented in sr_rt.c. The search algorithm is a 
simple traversal of the small routing table linked list. 

Buffering is implemented in sr_buffer.c/h. The buffer is one doubly linked 
list for all interfaces. A fixed sized array is used to actually store the 
data - this is much more stable than using malloc. The array is dimensioned
to be about twice the size of the maximum number of buffered packets needed
in practical experiments where memory was dynamically allocated with malloc.

The sr_router.c/h files tie together all of these elements. Some "background"
processing of arp and buffered packets is done in the while loop in sr_main.c. 

The system can be tested by changing to the router directory and running 
the sr_start.sh script and the sr_test.sh script. You should be able to 
access the test urls on the servers while the sr_test.sh script is running.
The system has been tested on long runs and should stay up indefinitely 
while under moderately heavy load.

