/**
 * by Cal Woodruff <cwoodruf@sfu.ca>
 */
#include "sr_arp.h"
/** 
    use a hash to index into the arp table 
    171.67.245.96/29 - addresses assigned 
    so if index into 256 entries or 171.67.245.96/24 
    we should be ok
*/
int sr_arp_get_index(uint32_t ip) {
    return ARP_MASK & ip;
}

/** 
    arp setter 
    given an ip and mac address
    refresh the arp entry
    @return the index into the arp entry
*/
int sr_arp_set_entry(sr_instance* sr, uint32_t ip, unsigned char* mac) {
	int index = sr_arp_get_index(ip);

        sr->arp_table[ index ].ip = ip;
	memset(
		sr->arp_table[ index ],
		mac,
		ETHER_ADDR_LEN
	);
	sr->arp_table[ index ].created = time();
	return index;
}
/**
    arp getter
    @return mac address
*/
unsigned char* sr_arp_get_entry(sr_instance* sr, uint32_t ip) {

	int index = sr_arp_get_index(ip);
	sr_arp* entry = sr->arp_table[ index ];

	if (entry->created - time() > ARP_TTL) {
		sr_arp_refresh(sr, ip);
	}
	return sr->arp_table[ index ].mac;
}
/**
    do an arp request to update a specific entry
*/
int sr_arp_refresh(sr_instance* sr, uint32_t ip) {
}

