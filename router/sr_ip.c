#include "sr_router.h"
unsigned short udp_sum_calc(
	unsigned short len_udp, 
	unsigned short src_addr[],
	unsigned short dest_addr[], 
	unsigned short padding, 
	unsigned short buff[]
);

void sr_icmp_time_exceeded(struct sr_ip_handle* h) {
}
void sr_icmp_handler(struct sr_ip_handle* h) {
}
void sr_ip_handler(struct sr_ip_handle* h) {
}

