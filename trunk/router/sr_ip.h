#ifndef SR_IP_H
#define SR_IP_H

/** icmp types */
#define ICMP_ECHO_REPLY 0x00
#define ICMP_UNREACHABLE 0x03
#define ICMP_PORT_UNAVAILABLE 0x03
#define ICMP_ECHO_REQUEST 0x08
#define ICMP_TIME_EXCEEDED 0x0b
#define ICMP_TRACEROUTE 0x1e

/** recommended here: http://www.maxi-pedia.com/time+to+live */
#define IP_MAX_HOPS 128

/** size in bytes of time exceeded data payload */
#define ICMP_TIMEOUT_SIZE 32

#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP 0x0001  
#endif

#ifndef IPPROTO_TCP
#define IPPROTO_TCP 0x0006  
#endif

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 0x0011
#endif

#define IPDATASIZE (VNSCMDSIZE-sizeof(struct sr_ethernet_hdr)-sizeof(struct ip))

/** strategy: make one big data structure that can be overlaid on a packet */

/** icmp structs - these are all 8 bytes */
struct sr_icmp_timeout
{
	uint32_t unused;
} __attribute__ ((packed)) ;

struct sr_icmp_unreachable
{
	uint16_t unused;
	uint16_t mtu;
} __attribute__ ((packed)) ;

struct sr_icmp_echo_reply
{
	uint16_t id;
	uint16_t sequence;
} __attribute__ ((packed)) ;

/** put all icmp structs together */
union sr_icmp_fields 
{
	struct sr_icmp_timeout timeout;
	struct sr_icmp_unreachable nothere;
	struct sr_icmp_echo_reply ping;
} __attribute__ ((packed)) ;

struct sr_icmp
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	union sr_icmp_fields fields;
	uint8_t data[IPDATASIZE-8];
} __attribute__ ((packed)) ;
/** end icmp stuff */

/** udp and tcp base headers */
struct sr_tcp /** 20 bytes */
{
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq;
	uint32_t ack;
	uint16_t flags; /* data offset:4, reserved:4, ecn:3, control bits:5 */
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent;
	uint8_t data[IPDATASIZE-20];
} __attribute ((packed)) ;

struct sr_udp /** 8 bytes */
{
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t len;
	uint16_t checksum;
	uint8_t data[IPDATASIZE-8];
} __attribute__ ((packed)) ;
/** end udp and tcp */

/** combine all the different types of payload together */
union sr_ip_payload
{
	struct sr_tcp tcp;
	struct sr_udp udp;
	struct sr_icmp icmp;
} __attribute ((packed)) ;

/** the big data structure */
struct sr_ip_packet
{
	struct sr_ethernet_hdr eth;
	struct ip ip;
	union sr_ip_payload d;
} __attribute ((packed)) ;
/** end cooked ip packet structures */

/** both udp and tcp use this pseudoheader to computer checksums; reserved = 0 always */
struct sr_pseudoheader {
	uint32_t src_ip, dest_ip;
	uint8_t reserved;
	uint8_t proto;
	uint16_t len;
};
	
#endif
