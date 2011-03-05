/**********************************************************************
 * file:  sr_router.c 
 * date:  Mon Feb 18 12:50:42 PST 2002  
 * Contact: casado@stanford.edu 
 *
 * Description:
 * 
 * This file contains all the functions that interact directly
 * with the routing table, as well as the main entry method
 * for routing.
 *
 **********************************************************************/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>


#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"

/*--------------------------------------------------------------------- 
 * Method: sr_init(void)
 * Scope:  Global
 *
 * Initialize the routing subsystem
 * 
 *---------------------------------------------------------------------*/

void sr_init(struct sr_instance* sr) 
{
    /* REQUIRES */
    assert(sr);

    printf("ROUTER: sr_init: zero out arp table and reset refresh timer\n");
    memset(sr->arp_table,0,sizeof(sr->arp_table)); 
    time(&sr->arp_lastrefresh);

} /* -- sr_init -- */



/*---------------------------------------------------------------------
 * Method: sr_handlepacket(uint8_t* p,char* interface)
 * Scope:  Global
 *
 * This method is called each time the router receives a packet on the
 * interface.  The packet buffer, the packet length and the receiving
 * interface are passed in as parameters. The packet is complete with
 * ethernet headers.
 *
 * Note: Both the packet buffer and the character's memory are handled
 * by sr_vns_comm.c that means do NOT delete either.  Make a copy of the
 * packet instead if you intend to keep it around beyond the scope of
 * the method call.
 *
 *---------------------------------------------------------------------*/

void sr_handlepacket(struct sr_instance* sr, 
        uint8_t * packet/* lent */,
        unsigned int len,
        char* interface/* lent */)
{
    struct sr_if*           iface = sr_get_interface(sr,interface);
    struct sr_ethernet_hdr* e_hdr = 0;
    struct sr_arphdr*       a_hdr = 0;
    struct ip*              ip_hdr = 0;
    struct sr_ip_handle     ip_handler;
    uint16_t                checksum;

    /* REQUIRES */
    assert(sr);
    assert(packet);
    assert(interface);

    e_hdr = (struct sr_ethernet_hdr*)packet;

    printf("ROUTER: Ethernet destination MAC: "); DebugMAC(e_hdr->ether_dhost); 
    printf(" ethernet source MAC: "); DebugMAC(e_hdr->ether_shost); printf("\n");

    switch (ntohs(e_hdr->ether_type)) 
    {
    case ETHERTYPE_IP:
        printf("ROUTER: IP packet\n");
        ip_hdr = (struct ip*)(packet + sizeof(struct sr_ethernet_hdr));

	if ((checksum = sr_ip_checksum((uint16_t*) ip_hdr, sizeof(struct ip)/2))) {
		printf("ROUTER: IP checksum failed (got %d) - aborting\n", checksum);
return;
	}

        memset(&ip_handler,0,sizeof(struct sr_ip_handle));
        ip_handler.sr = sr;
        ip_handler.eth = e_hdr;
        ip_handler.ip = ip_hdr = (struct ip*)
		(packet + sizeof(struct sr_ethernet_hdr));
	ip_handler.udp = (struct sr_udp*)
		(packet + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
	ip_handler.tcp = (struct sr_tcp*)
		(packet + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
	ip_handler.icmp = (struct sr_icmp*)
		(packet + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
	ip_handler.icmp_data = (uint8_t*)
		(packet + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct sr_icmp));
        ip_handler.packet = packet;
        ip_handler.len = len;
        ip_handler.iface = iface;

	/* transmogrify the data we are given */

        if (ip_hdr->ip_ttl <= 1) {
            printf("ROUTER: ttl expired!\n");
            sr_icmp_time_exceeded(&ip_handler);
        } else {
            switch(ip_hdr->ip_p)
            {
            case IPPROTO_ICMP:
                printf("ROUTER: ICMP protocol\n");
                sr_icmp_handler(&ip_handler);
            break;
            default:
                printf("ROUTER: Other IP protocol %d\n", ip_hdr->ip_p);
                sr_ip_handler(&ip_handler);
            }
break; /* not implemented yet */
	}

printf("ROUTER: ip len %X, udp len %X\n", 
	ntohs(ip_handler.ip->ip_len), ntohs(ip_handler.udp->len));
	/* then try and send it */
	sr_router_send(&ip_handler);

    break;
    case ETHERTYPE_ARP:
        a_hdr = (struct sr_arphdr*)(packet + sizeof(struct sr_ethernet_hdr));
        switch (ntohs(a_hdr->ar_op)) 
        {
        case ARP_REQUEST: 
            printf("ROUTER: ARP request - sending ARP reply\n");
            sr_arp_request_response(sr,packet,len,iface);
        break;
        case ARP_REPLY:
            printf("ROUTER: ARP reply - update ARP table\n");
            sr_arp_set(sr, a_hdr->ar_sip, a_hdr->ar_sha, interface);
        break;
        default:
            printf("ROUTER: ARP ERROR: don't know what %d is!\n", a_hdr->ar_op);
        }
    break;
    default:
        printf("ROUTER: ERROR: don't know what %d ethernet packet type is!\n", e_hdr->ether_type);
    }

}/* end sr_handlepacket */


/*--------------------------------------------------------------------- 
 * Method: sr_router_send
 *
 * figure out where we are sending and do basic sanity check
 * this is where you'd buffer packets should you not be able to send them
 *---------------------------------------------------------------------*/
void sr_router_send(struct sr_ip_handle* h) 
{
	struct sr_arp*  arp_entry;
	struct sr_rt*   sender;

	assert(h->sr);
	assert(h->ip->ip_src.s_addr);

	/* check if we can send on this interface */
	sender = sr_rt_find( h->sr, h->ip->ip_src.s_addr );

	arp_entry = sr_arp_get(h->sr, sender->gw.s_addr);

	if (arp_entry->tries > 5) {
		printf("ROUTER: interface %s is disconnected (tries %d) - aborting\n", 
			sender->interface, arp_entry->tries);
		return;
	}
	printf("ROUTER: attempting to send packet on interface %s\n", sender->interface);
	sr_send_packet(h->sr, h->packet, h->len, sender->interface);
}

