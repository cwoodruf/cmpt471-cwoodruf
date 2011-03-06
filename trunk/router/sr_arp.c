/**
 * by Cal Woodruff <cwoodruf@sfu.ca>
 *
 * Defines the arp table for the router using a very simple hash table based on ip.
 *
 * Since we are never going to make a router that will 
 * work with an infinite lan we can use a reasonably sized array
 * to store the lan information.
 *
 * In our case we use an array slightly bigger than the provisioned 
 * number of ip addresses for the subnet we are on. See LAN_SIZE in sr_arp.h.
 *
 * Accessing the array is done via a hash function. Given that the first 3
 * octets of the ip addresses on the lan are all the same this can be simply:
 *
 * ip & LAN_SIZE - 1
 *
 * for a lan of LAN_SIZE
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
 * check if we need to spam the LAN with arp broadcasts
 */
void sr_arp_check_refresh(struct sr_instance* sr) 
{
	time_t t, age, refreshage;
	int i;
	struct sr_arp* entry;

	assert(sr);

	refreshage = time(&t) - sr->arp_lastrefresh;
	/* printf("ARP: running check refresh refresh age: %lds\n", refreshage); */

	if (refreshage >= ARP_CHECK_EVERY) {
		for (i=0; i<LAN_SIZE; i++) {
			entry = &sr->arp_table[i];
			if (!entry->ip) continue;

			age = t - entry->created;
			printf("ARP: Entry %i aged %lds (ttl %ds)\n", i, age, ARP_TTL);
			if (age <= ARP_TTL) continue;

			printf("ARP: Updating ");
			sr_arp_print_entry(i,*entry);

			entry->tries++;
			sr_arp_refresh(
				sr,
				entry->ip,
				entry->iface->name
			);
		}
		sr->arp_lastrefresh = t;
	}
}
/*---------------------------------------------------------------------------*/
/** 
    use a hash to index into the arp table 
    171.67.245.96/29 - addresses assigned 
    so if index into 256 entries or 171.67.245.96/24 
    we should not get collisions with this trivial hashing function
*/
uint8_t sr_arp_get_index(uint32_t ip) 
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
struct sr_arp* sr_arp_set(struct sr_instance* sr, uint32_t ip, unsigned char* mac, struct sr_if* iface) 
{
	int index = sr_arp_get_index(ip);
	struct sr_arp* entry = &sr->arp_table[ index ];

	assert(sr);
	assert(ip);
	assert(mac);
	assert(iface);

	memset((void *)entry, 0, sizeof(struct sr_arp));
        entry->ip = ip;
	memcpy(
		entry->mac,
		mac,
		ETHER_ADDR_LEN
	);
	entry->iface = iface;
	entry->tries = 0;
	time(&entry->created);

	printf("ARP: Created entry %d\n",index);
	/* sr_arp_print_table(sr); */

	return entry;
}
/*---------------------------------------------------------------------------*/
/**
    arp getter
    @return mac address
*/
struct sr_arp* sr_arp_get(struct sr_instance* sr, uint32_t ip) 
{
	int index = sr_arp_get_index(ip);

	assert(sr);
	assert(ip);
	assert(sr->arp_table[ index ].ip = ip);
	
	return &sr->arp_table[ index ];
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
	if (!iface) {
		printf("ARP: sr_arp_refresh: interface %s not found: aborting\n", interface); 
		return;
	}

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
	a_hdr->ar_op = htons(ARP_REQUEST);
	memcpy(a_hdr->ar_sha, iface->addr, ETHER_ADDR_LEN);
	a_hdr->ar_sip = iface->ip;
	memcpy(a_hdr->ar_tha, sr_arp_get(sr,ip), ETHER_ADDR_LEN);
	a_hdr->ar_tip = ip;

	/* send the packet and cross our fingers! */
	sr_send_packet(sr, packet, sizeof(packet), interface);
}
/*---------------------------------------------------------------------------*/
/**
 * recycle an arp request packet as a response
 */
void sr_arp_request_response(
	struct sr_instance* sr,
	uint8_t* packet,
	unsigned int len,
	struct sr_if* iface
) {
	struct sr_ethernet_hdr* e_hdr = 0;
	struct sr_arphdr*       a_hdr = 0;
	uint32_t                tmp_ip;

	assert(sr);
	assert(packet);
	assert(len);
	assert(iface->ip);

	e_hdr = (struct sr_ethernet_hdr*)packet;
	a_hdr = (struct sr_arphdr*)(packet + sizeof(struct sr_ethernet_hdr));

	/* check to see if the packet is for us */
	if (iface->ip != a_hdr->ar_tip) {
		printf("ARP: Arp request is not for us - aborting!");
		return;
	}

	/* recycle the packet as a response */
	memcpy(e_hdr->ether_dhost, e_hdr->ether_shost, ETHER_ADDR_LEN);
	memcpy(e_hdr->ether_shost, iface->addr, ETHER_ADDR_LEN);
	a_hdr->ar_op = htons(ARP_REPLY);

	/* basically swap but add our ethernet address instead of the broadcast */
	memcpy(a_hdr->ar_tha, a_hdr->ar_sha, ETHER_ADDR_LEN);
	memcpy(a_hdr->ar_sha, iface->addr, ETHER_ADDR_LEN);

	/* swap IPs as well of course */
	tmp_ip = a_hdr->ar_sip;
	a_hdr->ar_sip = a_hdr->ar_tip; 
	a_hdr->ar_tip = tmp_ip; 
	sr_send_packet(sr, (uint8_t*)packet, len, iface->name);
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
        printf("ARP: *warning* Routing table empty \n");
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
/*---------------------------------------------------------------------------*/
/** 
 * scan the arp array and print any entries you find
 */
void sr_arp_print_table(struct sr_instance* sr) 
{
	int i;
	printf("ARP: Current arp entries out of a total of %d:\n",LAN_SIZE);
	for (i=0; i<LAN_SIZE; i++) {
		if (sr->arp_table[i].ip) sr_arp_print_entry(i,sr->arp_table[i]);
	}
	printf("ARP: End of arp table.\n");
}
/*---------------------------------------------------------------------------*/
/**
 * format and print an arp entry
 */
void sr_arp_print_entry(int i, struct sr_arp entry) 
{
	time_t t, age;
	struct in_addr pr_ip;

	pr_ip.s_addr = entry.ip;
	age = time(&t) - entry.created;

	printf("ARP: table entry %d ip %s mac ", i, inet_ntoa(pr_ip));
	DebugMAC(entry.mac);
	printf(" tries %d age %lds created %s ", entry.tries, age, ctime(&entry.created));
}
