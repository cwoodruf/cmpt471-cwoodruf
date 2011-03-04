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

        memset(&ip_handler,0,sizeof(struct sr_ip_handle));
        ip_handler.sr = sr;
        ip_handler.e_hdr = e_hdr;
        ip_handler.ip_hdr = ip_hdr = (struct ip*)(packet + sizeof(struct sr_ethernet_hdr));
        ip_handler.packet = packet;
        ip_handler.len = len;
        ip_handler.iface = iface;

        if (ip_hdr->ip_ttl <= 1) {
            printf("ROUTER: ttl expired!\n");
            sr_icmp_time_exceeded(&ip_handler);
            break;
        }
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

}/* end sr_ForwardPacket */


/*--------------------------------------------------------------------- 
 * Method:
 *
 *---------------------------------------------------------------------*/
