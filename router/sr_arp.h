/**
 * by Cal Woodruff <cwoodruf@sfu.ca>
 */
#ifndef SR_ARP_H
#define SR_ARP_H

#include <sys/time.h>
#include <stdint.h>
#include "sr_protocol.h"
#include "sr_if.h"


/** data structure for an arp entry */
struct sr_arp {
	uint32_t ip;
	unsigned char mac[ETHER_ADDR_LEN];
	char interface[sr_IFACE_NAMELEN];
	time_t created;
};

/** instead of having clunky linked lists use an array that is probably large enough for most LANs */
#define LAN_SIZE 256

/** mask for arp table hash function */
#define ARP_MASK 0xFF

/** broadcast address for arp requests */
#define ARP_BROADCAST 0xFFFFFFFFFFFF
/** seconds arp entry is allowed to be valid: normally this is 10 min to 4 hours depending on system */
#define ARP_TTL 60

#endif

