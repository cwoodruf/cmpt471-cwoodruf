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
	struct sr_ip_packet* p = h->pkt;

	/* but then copy the ip header and 64 bits of the original datagram */
	memcpy(data, (uint8_t*) &p->ip, ICMP_TIMEOUT_SIZE);

	/* swap the sender and receiver ethernet mac addresses */
	memcpy(shost, p->eth.ether_dhost, ETHER_ADDR_LEN);
	memcpy(p->eth.ether_dhost, p->eth.ether_shost, ETHER_ADDR_LEN);
	memcpy(p->eth.ether_shost, shost, ETHER_ADDR_LEN);

	/* change the protocol to ICMP and reset data in the ip header */
	/* ihl - length of header in bytes */
	if (p->ip.ip_hl != 5) {
		p->ip.ip_hl &= 0; /* clear nibble */
		p->ip.ip_hl |= (1 << 2) + 1; /* make 0101 in binary */
	}
	p->ip.ip_off = 0;
	p->ip.ip_ttl = IP_MAX_HOPS;
	p->ip.ip_p = IPPROTO_ICMP;
	p->ip.ip_sum = 0;

	/* swap the ip addresses */
	s_ip = p->ip.ip_dst.s_addr;
	p->ip.ip_dst.s_addr = p->ip.ip_src.s_addr;
	p->ip.ip_src.s_addr = s_ip;

	/* recalculate the length */
	p->ip.ip_len = htons(20 /*ip*/ + 8 /*icmp*/ + 32 /*data*/);

	/* now that we have everything in the ip header recompute the checksum */
	p->ip.ip_sum = sr_ip_checksum((uint16_t*) &p->ip, sizeof(struct ip)/2);
	printf(
		"IP: time exceeded: calculated checksum %X, recalculated %X (should be 0)\n", 
		ntohs(p->ip.ip_sum), 
		sr_ip_checksum((uint16_t*) &p->ip, sizeof(struct ip)/2)
	);

	/* create the icmp packet */
	p->d.icmp.type = ICMP_UNREACHABLE;
	p->d.icmp.code = ICMP_PORT_UNAVAILABLE;

	/* 0 = no checksum */
	/* icmp messages don't have to have checksums: to say you have a checksum of 0 use all 1s */
	p->d.icmp.checksum = 0;

	/* clear data from unused - only field for this type */
	p->d.icmp.fields.timeout.unused = 0;

	/* add the data from the original datagram */
	memcpy(p->d.icmp.data, data, ICMP_TIMEOUT_SIZE);

	p->d.icmp.checksum = sr_ip_checksum((uint16_t*) &p->d.icmp, (ICMP_TIMEOUT_SIZE+8)/2);

	/* recalculate the size of our packet */
	h->len = sizeof(struct sr_ethernet_hdr) + ntohs(p->ip.ip_len);
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
