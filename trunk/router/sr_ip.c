/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 *
 * implementation of ip specific routines esp checksums
 */
#include <assert.h>
#include "sr_router.h"
/**
 * what to do if we get a packet that is too old (eg from traceroute)
 */
void sr_icmp_time_exceeded(struct sr_ip_handle* h) 
{
}
/**
 * what to do if we get an icmp request
 */
void sr_icmp_handler(struct sr_ip_handle* h) 
{
}
/**
 * what to do with other ip packets
 */
void sr_ip_handler(struct sr_ip_handle* h) 
{
	assert(h);
	printf(
		"IP: ip header checksum is %d\n", 
		sr_ip_checksum((uint16_t*) h->ip_hdr, sizeof(struct ip)/2)
	);
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
