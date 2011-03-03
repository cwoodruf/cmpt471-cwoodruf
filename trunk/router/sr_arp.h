/**
 * by Cal Woodruff <cwoodruf@sfu.ca>
 */
#ifndef __SR_ARP__
#define __SR_ARP__

#include <time.h>
#include <stdint.h>

#include "sr_protocol.h"

/** data structure for an arp entry */
typedef struct {
	uint32_t ip;
	unsigned char mac[ETHER_ADDR_LEN];
	time_t created;
} sr_arp;

/** instead of having clunky linked lists use an array that is probably large enough for most LANs */
#define LAN_SIZE 256

/** mask for arp table hash function */
#define ARP_MASK 0xFF

/** seconds arp entry is allowed to be valid: normally this is 10 min to 4 hours depending on system */
#define ARP_TTL 60

/** use a hash to index into the arp table */
int sr_arp_get_index(uint32_t ip);

/** arp getter and setter */
int sr_arp_set(struct sr_instance* sr, uint32_t ip, unsigned char* mac);
unsigned char* sr_arp_get(struct sr_instance* sr, uint32_t ip);

/** do an arp broadcast to get an updated entry for an ip */
void sr_arp_refresh(struct sr_instance* sr, uint32_t ip);

/** may want a method to check all entries */

#endif

