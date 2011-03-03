/**
 * by Cal Woodruff <cwoodruf@sfu.ca>
 */
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
/*---------------------------------------------------------------------------*/
/** 
    use a hash to index into the arp table 
    171.67.245.96/29 - addresses assigned 
    so if index into 256 entries or 171.67.245.96/24 
    we should not get collisions with this trivial hashing function
*/
int sr_arp_get_index(uint32_t ip) 
{
	assert(ip);
	return ARP_MASK & ntohl(ip);
}
/*---------------------------------------------------------------------------*/
/** 
    arp setter 
    given an ip and mac address
    refresh the arp entry
    @return the index into the arp entry
*/
int sr_arp_set(struct sr_instance* sr, uint32_t ip, unsigned char* mac, char* interface) 
{

	int index = sr_arp_get_index(ip);
	struct sr_arp* entry = &sr->arp_table[ index ];

	assert(sr);
	assert(ip);
	assert(mac);

	memset((void *)entry, 0, sizeof(struct sr_arp));
        entry->ip = ip;
	memcpy(
		entry->mac,
		mac,
		ETHER_ADDR_LEN
	);
	strncpy(entry->interface, interface, sr_IFACE_NAMELEN);
	time(&entry->created);
	return index;
}
/*---------------------------------------------------------------------------*/
/**
    arp getter
    @return mac address
*/
unsigned char* sr_arp_get(struct sr_instance* sr, uint32_t ip) 
{

	int index = sr_arp_get_index(ip);
	time_t t;

	assert(sr);
	assert(ip);
	assert(sr->arp_table[ index ].ip = ip);

	if (sr->arp_table[ index ].created - time(&t) > ARP_TTL) {
		sr_arp_refresh(sr, ip, sr->arp_table[ index ].interface);
	}
	return sr->arp_table[ index ].mac;
}
/*---------------------------------------------------------------------------*/
/**
    do an arp request to update a specific entry
*/
void sr_arp_refresh(struct sr_instance* sr, uint32_t ip, char* interface) 
{
	
	int i;
	uint8_t packet[
			sizeof(struct sr_ethernet_hdr) + 
			sizeof(struct sr_arphdr)
	];
	struct sr_ethernet_hdr* e_hdr = 
			(struct sr_ethernet_hdr*)packet;
	struct sr_arphdr* a_hdr = 
			(struct sr_arphdr*)(packet + sizeof(struct sr_ethernet_hdr));
	struct sr_if* iface = 
			sr_get_interface(sr,interface);

	assert(sr);
	assert(ip);
	assert(interface);
	assert(iface);
	/* ethernet header for broadcast */
	memset((void *)packet, 0, sizeof(packet));
	for (i=0; i<ETHER_ADDR_LEN; i++) {
		e_hdr->ether_dhost[i] = 0xFF;
		e_hdr->ether_shost[i] = iface->addr[i]; 
	} 
	e_hdr->ether_type = htons(ETHERTYPE_ARP);

        /* arp message for broadcast */
	a_hdr->ar_hrd = htons(ARPHDR_ETHER);
	a_hdr->ar_pro = htons(ETHERTYPE_IP);
	a_hdr->ar_hln = ETHER_ADDR_LEN;
	a_hdr->ar_pln = sizeof(uint32_t);
	a_hdr->ar_op = ARP_REQUEST;
	memcpy(a_hdr->ar_sha, iface->addr, ETHER_ADDR_LEN);
	a_hdr->ar_sip = iface->ip;
	memcpy(a_hdr->ar_tha, sr_arp_get(sr,ip), ETHER_ADDR_LEN);
	a_hdr->ar_tip = ip;

	/* send the packet and cross our fingers! */
	sr_send_packet(sr, packet, sizeof(packet), interface);
}
/*---------------------------------------------------------------------------*/
/**
    scan the routing table (taken from sr_rt.c's sr_print_routing_table)
    and send arp requests for any relevant ip addresses we find
    in this case gateways as these are the only hosts we actually communicate with

    TODO one problem with this implementation is that we don't check 
    if we've already made a request to this gw
*/
void sr_arp_scan(struct sr_instance* sr) 
{
    struct sr_rt* rt_walker = 0;

    assert(sr);
    if(sr->routing_table == 0)
    {
        printf(" *warning* Routing table empty \n");
        return;
    }

    rt_walker = sr->routing_table;
    
    sr_arp_refresh(sr, rt_walker->gw.s_addr, rt_walker->interface);
    while(rt_walker->next)
    {
        rt_walker = rt_walker->next; 
        sr_arp_refresh(sr, rt_walker->gw.s_addr, rt_walker->interface);
    }

} 

