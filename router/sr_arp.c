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
	assert(ip);
	return ARP_MASK & ntohl(ip);
}

/** 
    arp setter 
    given an ip and mac address
    refresh the arp entry
    @return the index into the arp entry
*/
int sr_arp_set(struct sr_instance* sr, uint32_t ip, unsigned char* mac) {

	int index = sr_arp_get_index(ip);

	assert(sr);
	assert(ip);
	assert(mac);

	memset(sr->arp_table[ index ], 0, sizeof(sr_arp));
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
unsigned char* sr_arp_get(struct sr_instance* sr, uint32_t ip) {

	int index = sr_arp_get_index(ip);
	sr_arp* entry = sr->arp_table[ index ];

	assert(sr);
	assert(ip);
	assert(sr->arp_table[ index ]);

	if (entry->created - time() > ARP_TTL) {
		sr_arp_refresh(sr, ip);
	}
	return sr->arp_table[ index ].mac;
}
/**
    do an arp request to update a specific entry
*/
void sr_arp_refresh(struct sr_instance* sr, uint32_t ip, char* interface) {
	
	int index = sr_arp_get_index(ip);
	uint8_t packet[
			sizeof(sr_ether_hdr) + 
			sizeof(sr_arp_hdr)
	];
	struct sr_ether_hdr* e_hdr = 
			(struct sr_ether_hdr*)packet;
	struct sr_arphdr* a_hdr = 
			(struct sr_arphdr*)(packet + sizeof(sr_ether_hdr));
	struct sr_if* iface = 
			sr_get_interface(sr,interface);

	assert(sr);
	assert(ip);
	assert(interface);

	/* ethernet header for broadcast */
	memset(packet,0,sizeof(packet));
	memcpy(e_hdr->ether_dhost, ARP_BROADCAST, ETHER_ADDR_LEN);
	memcpy(e_hdr->ether_shost, iface->addr, ETHER_ADDR_LEN);
	e_hdr->ether_type = ETHERTYPE_ARP;

        /* arp message for broadcast */
	a_hdr->ar_hrd = htons(ARPHDR_ETHER);
	a_hdr->ar_pro = htons(ETHERTYPE_IP);
	a_hdr->ar_hln = ETHER_ADDR_LEN;
	a_hdr->ar_pln = sizeof(uint32_t);
	a_hdr->ar_op = ARP_REQUEST;
	memcpy(a_hdr->ar_sha, if_mac, ETHER_ADDR_LEN);
	a_hdr->ar_sip = if_ip;
	memcpy(a_hdr->ar_tha, sr_arp_get(sr,ip), ETHER_ADDR_LEN);
	a_hdr->ar_tip = ip;

	/* send the packet and cross our fingers! */
	sr_send_packet(sr, packet, interface);
}

