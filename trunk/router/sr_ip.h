#ifndef SR_IP_H
#define SR_IP_H

/** icmp types */
#define ICMP_ECHO_REPLY 0
#define ICMP_UNREACHABLE 3
#define ICMP_ECHO_REQUEST 8
#define ICMP_TIME_EXCEEDED 11
#define ICMP_TRACEROUTE 30

/** recommended here: http://www.maxi-pedia.com/time+to+live */
#define IP_MAX_HOPS 128

/** size in bytes of time exceeded data payload */
#define ICMP_TIMEOUT_SIZE 32

#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP 0x0001  
#endif

/** minimal icmp header: http://www.networksorcery.com/enp/protocol/icmp.htm */
struct sr_icmp_timeout
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t unused;
} __attribute__ ((packed)) ;

struct sr_icmp_unreachable
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t unused;
	uint16_t mtu;
} __attribute__ ((packed)) ;

struct sr_icmp_echo_reply
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t sequence;
} __attribute__ ((packed)) ;

#ifndef IPPROTO_TCP
#define IPPROTO_TCP 0x0006  
#endif

/** minimal tcp header: http://www.networksorcery.com/enp/protocol/tcp.htm */
struct sr_tcp
{
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq;
	uint32_t ack;
	uint16_t flags; /* data offset:4, reserved:4, ecn:3, control bits:5 */
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent;
} __attribute ((packed)) ;

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 0x0011
#endif

/** minimal udp header: http://www.networksorcery.com/enp/protocol/udp.htm */
struct sr_udp 
{
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t len;
	uint16_t checksum;
} __attribute__ ((packed)) ;

/** both udp and tcp use this pseudoheader to computer checksums; reserved = 0 always */
struct sr_pseudoheader {
	uint32_t src_ip, dest_ip;
	uint8_t reserved;
	uint8_t proto;
	uint16_t len;
};
	
#endif
