Cmpt471 Spring 2011 SFU Surrey

Author Cal Woodruff <cwoodruf@sfu.ca> 301013983

Design:

The arp cache is checked every 10 seconds and is refreshed every 60 seconds.
Arp data is held in an array. The IP address of the destination is hashed
to find arp data for that IP. Hash collisions are virtually non-existent given 
that the subnet is set up with all IP addresses sharing the same 29 bit CIDR 
prefix. (Nevertheless, if a hash collision were to occur, the algorithm for 
handling it is to count through the arp array until an available array 
index is found.) Arp code is found in sr_arp.c and sr_arp.h.

Packet processing is implemented in sr_ip.c and sr_ip.h. A single c structure 
overlay was used to access headers and data in the packets. Unions are used to
handle cases where the packet data must be handled differently based on the
type of packet (IP, ICMP, ARP).

The routing of packets is implemented in sr_rt.c. The search algorithm is a 
simple traversal of the small routing table linked list. 

Buffering is implemented in sr_buffer.c and sr_buffer.h. The buffer is one 
doubly linked list for all interfaces. A fixed sized array is used to actually 
store the data - this is much more stable than using malloc. The array is 
dimensioned such that it is twice the size of the maximum number of buffered 
packets observed in practical experiments where memory was dynamically allocated 
with malloc. The main rationale for this design is flexibility and stability.
Buffered packets are dropped if they cannot be sent after 6 seconds.

To make the original code more efficient and less prone to crashes some 
modifications were made. The sr_vns_comm.c functions "sr_handle_auth_request" 
and "sr_read_from_server_expect" were changed so that packets were stored in 
statically allocated fixed sized arrays rather than arrays allocated with 
malloc. Also, an alternate method for accessing interface data was 
implemented (see sr_if.c and sr_if.h). The new function sr_if_name2iface 
uses a simple hash array to access interface data much like the arp 
implementation. This function replaces sr_get_interface. 

There are some limitations of the new interface access implementation. 
Interfaces are assumed to be of the form "ethN" where N is an integer between
0 and 255. "eth" can be any 3 characters. These limitations do not affect 
actual use with the vns network.

The sr_router.c and sr_router.h files tie together packet processing,
routing and sending. Some "background" processing of arp and buffered 
packets is initialized in the while loop in sr_main.c. 

The system can be tested by changing to the "router" directory and running 
the "sr_start.sh" script and the "sr_test.sh" script. You should be able to 
access the test urls on the servers while the sr_test.sh script is running.
Test that the vns system has started by checking the log files produced by
sr_start.sh before starting sr_test.sh. The system has been tested on long 
runs and seemed to be able to stay up indefinitely while under moderately 
heavy load.

