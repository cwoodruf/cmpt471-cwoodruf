#ifndef SR_IP_H
#define SR_IP_H

/** icmp types */
#define ICMP_ECHO_REPLY 0
#define ICMP_UNREACHABLE 3
#define ICMP_ECHO_REQUEST 8
#define ICMP_TIME_EXCEEDED 11
#define ICMP_TRACEROUTE 30

struct sr_icmp {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	unsigned char* data;
};

/** both udp and tcp use this pseudoheader to computer checksums; reserved = 0 always */
struct sr_pseudoheader {
	uint32_t src_ip, dest_ip;
	uint8_t reserved;
	uint8_t proto;
	uint16_t len;
};
	
#endif
