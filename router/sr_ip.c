/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 *
 * implementation of ip specific routines esp checksums
 */
#include <assert.h>
#include <string.h>
#include "sr_router.h"
/**
 * what to do if we get a packet that is too old (eg from traceroute)
 * fairly simple send a time exceeded icmp packet back where this came from
 */
void sr_icmp_time_exceeded(struct sr_ip_handle* h) 
{
	uint8_t  shost[ETHER_ADDR_LEN];
	uint8_t  data[ICMP_TIMEOUT_SIZE];
	uint32_t s_ip;

	/* define where the icmp header starts in the packet */
	h->icmp = (struct sr_icmp*) (h->ip + sizeof(h->ip));

	/* get data for the time exceeded message - first 4 bytes are not used */
	memset(data, 0, 4);
	/* but then copy the ip header and 64 bits of the original datagram */
	memcpy(data+4, h->ip, 28);

	/* swap the sender and receiver ethernet mac addresses */
	memcpy(shost, h->eth->ether_dhost, ETHER_ADDR_LEN);
	memcpy(h->eth->ether_dhost, h->eth->ether_shost, ETHER_ADDR_LEN);
	memcpy(h->eth->ether_shost, shost, ETHER_ADDR_LEN);

	/* change the protocol to ICMP and reset data in the ip header */
	h->ip->ip_hl = htons(5);
	h->ip->ip_p = htons(IPPROTO_ICMP);
	h->ip->ip_off = 0;
	h->ip->ip_ttl = htons(IP_MAX_HOPS);
	h->ip->ip_sum = 0;

	/* swap the ip addresses */
	s_ip = h->ip->ip_dst.s_addr;
	h->ip->ip_dst.s_addr = h->ip->ip_src.s_addr;
	h->ip->ip_src.s_addr = s_ip;

	/* recalculate the length */
	h->ip->ip_len = htons(sizeof(struct ip) + sizeof(struct sr_icmp) + ICMP_TIMEOUT_SIZE);

	/* now that we have everything in the ip header recompute the checksum */
	h->ip->ip_sum = htons(sr_ip_checksum((uint16_t*) h->ip, sizeof(struct ip)/2));
	printf(
		"IP: time exceeded: calculated checksum %d, recalculated as %d\n", 
		h->ip->ip_sum, 
		sr_ip_checksum((uint16_t*) h->ip, sizeof(struct ip)/2)
	);

	/* make an ip ICMP header */
	h->icmp->type = ICMP_TIME_EXCEEDED;

	/* code 1 = fragment reassembly timed out - which we aren't implementing */
	/* code 0 = time to live went to 0 */
	h->icmp->code = 0; 

	/* 0 = no checksum */
	/* icmp messages don't have to have checksums: to say you have a checksum of 0 use all 1s */
	h->icmp->checksum = 0;
	
	/* add the data from the original datagram */
	memcpy((uint8_t*) (&h->icmp + sizeof(h->icmp)), (uint8_t*) data, ICMP_TIMEOUT_SIZE);

	/* recalculate the size of our packet */
	h->len = sizeof(struct sr_ethernet_hdr) + h->ip->ip_len;
}
/**
 * what to do if we get an icmp request
 */
void sr_icmp_handler(struct sr_ip_handle* h) 
{
	assert(h);
}
/**
 * what to do with other ip packets
 */
void sr_ip_handler(struct sr_ip_handle* h) 
{
	assert(h);
}
/**
 * do a basic checksum calculation
 *
 * this routine is based on:
 * http://www.netrino.com/Embedded-Systems/How-To/Additive-Checksums
 * by Michael Barr (http://www.embeddedgurus.net/barr-code/)
 *
 * see also http://www.faqs.org/rfcs/rfc1071.html 
 * for examples of how to compute these checksums
 * 
 * we assume there are an even number of bytes
 * which means we need to pad anything that is 
 * odd before sending it to this function
 *
 * when checking an incoming packet this should return 0xFFFF 
 * if the checksum is correct
 * 
 * when making the checksum for a header with an embedded checksum 
 * the embedded checksum in the header is set to 0
 * 
 */ 
uint16_t sr_ip_checksum(uint16_t const data[], uint16_t words) 
{
	uint32_t sum = 0;

	/* add up each 16 bit word */
	while(words-- > 0) {
		sum += *(data++);
	}

	/* squash sum back into 16 bits: compute 1s compliment with carries */
	while (sum >> 16) { sum = (sum >> 16) + (sum & 0xFFFF); }

	/* invert the answer */
	return ((uint16_t) ~sum);
}
