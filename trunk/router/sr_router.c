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

    printf("sr_init: zero out arp table\n");
    memset(sr->arp_table,0,sizeof(sr->arp_table)); 
    printf("sr_init: TODO scan routing table for local hosts and make arp requests for them\n");

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
    uint32_t                tmp_ip;

    /* REQUIRES */
    assert(sr);
    assert(packet);
    assert(interface);

    e_hdr = (struct sr_ethernet_hdr*)packet;

    printf("Ethernet destination MAC: %lx ",e_hdr->ether_dhost); DebugMAC(e_hdr->ether_dhost); 
    printf(" ethernet source MAC: "); DebugMAC(e_hdr->ether_shost); printf("\n");

    switch (ntohs(e_hdr->ether_type)) {
    case ETHERTYPE_IP:
        printf("IP packet\n");
    break;
    case ETHERTYPE_ARP:
        a_hdr = (struct sr_arphdr*)(packet + sizeof(struct sr_ethernet_hdr));
        memcpy(e_hdr->ether_dhost, e_hdr->ether_shost, ETHER_ADDR_LEN);
        memcpy(e_hdr->ether_shost, iface->addr, ETHER_ADDR_LEN);

        switch (ntohs(a_hdr->ar_op)) {
        case ARP_REQUEST: 
            printf("ARP request - sending ARP reply\n");
            a_hdr->ar_op = htons(ARP_REPLY);
            /* basically swap but add our ethernet address instead of the broadcast */
    	    memcpy(a_hdr->ar_tha, a_hdr->ar_sha, ETHER_ADDR_LEN);
    	    memcpy(a_hdr->ar_sha, iface->addr, ETHER_ADDR_LEN);
            /* swap IPs as well of course */
            tmp_ip = a_hdr->ar_sip;
            a_hdr->ar_sip = a_hdr->ar_tip; 
            a_hdr->ar_tip = tmp_ip; 
            sr_send_packet(sr, (uint8_t*)packet, len, interface);
        break;
        case ARP_REPLY:
            printf("ARP reply - update ARP table\n");
        break;
        default:
            printf("ARP ERROR: don't know what %d is!\n", a_hdr->ar_op);
        }
        printf("Ethernet destination: ");
        DebugMAC(e_hdr->ether_dhost);
        printf(" ethernet source MAC: ");
        DebugMAC(e_hdr->ether_shost);
        printf("\n");
        printf(" arp packet: sender ");
        DebugMAC(a_hdr->ar_tha);
        printf(" target ");
        DebugMAC(a_hdr->ar_sha);
        printf("\n");
    break;
    default:
        printf("ERROR: don't know what %d ethernet packet type is!\n", e_hdr->ether_type);
    }

}/* end sr_ForwardPacket */


/*--------------------------------------------------------------------- 
 * Method:
 *
 *---------------------------------------------------------------------*/
