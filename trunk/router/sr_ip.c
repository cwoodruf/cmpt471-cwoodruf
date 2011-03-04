#include "sr_router.h"
void sr_icmp_time_exceeded(struct sr_ip_handle* h) 
{
}
void sr_icmp_handler(struct sr_ip_handle* h) 
{
}
void sr_ip_handler(struct sr_ip_handle* h) 
{
}
/**
 * this routine is based on:
 * http://www.netrino.com/Embedded-Systems/How-To/Additive-Checksums
 * by Michael Barr (http://www.embeddedgurus.net/barr-code/)
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
 * see also http://www.faqs.org/rfcs/rfc1071.html 
 * for examples of how to compute these checksums
 * 
 */ 
uint16_t sr_ip_checksum(uint16_t const data[], uint16_t words) 
{
	uint32_t sum = 0;

	/* add up each 16 bit word */
	while(words-- > 0) {
		sum += *(data++);
	}

	/* compute 1s compliment with carries */
	sum = (sum >> 16) + (sum & 0xFFFF);

	/* invert the answer */
	return ((unsigned short) ~sum);
}
