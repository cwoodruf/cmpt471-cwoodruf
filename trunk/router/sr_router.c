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


#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_buffer.h"

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
    /* struct sr_if*           iface = sr_get_interface(sr,interface); */
    struct sr_if*           iface = sr_if_name2iface(sr,interface); /* faster */
    struct sr_if*           ipif; /* used to test where traffic is going */
    struct sr_ethernet_hdr* e_hdr = 0;
    struct sr_arphdr*       a_hdr = 0;
    struct ip*              ip = 0;
    struct sr_ip_handle     ip_handler;
    uint16_t                checksum;
    int                     send_result;
    time_t                  t;

    /* REQUIRES */
    assert(sr);
    assert(packet);
    assert(interface);

    e_hdr = (struct sr_ethernet_hdr*)packet;

    time(&t);
    Debug("ROUTER: %s",ctime(&t));
/*    Debug("ROUTER: Ethernet destination MAC: "); DebugMAC(e_hdr->ether_dhost); */
/*    Debug(" ethernet source MAC: "); DebugMAC(e_hdr->ether_shost); Debug("\n"); */

    ip = (struct ip*) (packet + sizeof(struct sr_ethernet_hdr));
    switch (ntohs(e_hdr->ether_type)) 
    {
    case ETHERTYPE_IP:
        /* this doesn't work because they send packets for machines not in our subnet 
           with addresses that are valid for our subnet block */
        if (!(
              (ip->ip_dst.s_addr & sr->subnet & sr->mask) == sr->subnet || 
              (ip->ip_src.s_addr & sr->subnet & sr->mask) == sr->subnet
             )
        ) { 
            Debug("ROUTER: IP packet not for our subnet ");
            Debug("src %s ", inet_ntoa(ip->ip_src));
            Debug("dst %s ", inet_ntoa(ip->ip_src));
            Debug("\n");
            return;
        }
        Debug("ROUTER: IP packet\n");
        if ((checksum = sr_ip_checksum((uint16_t*) ip, (ip->ip_hl*4)))) {
            Debug("ROUTER: IP checksum failed (got %X) - aborting\n", checksum);
            return;
        }

        memset(&ip_handler,0,sizeof(struct sr_ip_handle));
        ip_handler.sr = sr;
        ip_handler.raw = packet;
        ip_handler.raw_len = len;
        ip_handler.pkt = (struct sr_ip_packet*) packet;
        ip_handler.len = len;
        ip_handler.iface = iface;

        /* transmogrify the data we are given and send if we are successful */
        if (ip->ip_ttl <= 1) {
            Debug("ROUTER: ttl expired!\n");
            if (!sr_icmp_unreachable(&ip_handler)) return;

        } else if (ip->ip_p == IPPROTO_ICMP) {
            Debug("ROUTER: ICMP protocol\n");
            if (!sr_icmp_handler(&ip_handler)) return;

        } else if ((ipif = sr_if_ip2iface(sr, ip->ip_dst.s_addr))) {
            Debug("ROUTER: destination is interface %s\n", ipif->name);
            if (!sr_icmp_unreachable(&ip_handler)) return;

        } else {
            Debug("ROUTER: IP protocol %d\n", ip->ip_p);
            if (!sr_ip_handler(&ip_handler)) return;
        }

        /* then try and send it */
        send_result = sr_router_send(&ip_handler);
        Debug("ROUTER: send result %d\n", send_result);

    break;
    case ETHERTYPE_ARP:
        a_hdr = (struct sr_arphdr*)(packet + sizeof(struct sr_ethernet_hdr));
        switch (ntohs(a_hdr->ar_op)) 
        {
        case ARP_REQUEST: 
            Debug("ROUTER: ARP request - sending ARP reply\n");
            sr_arp_request_response(sr,packet,len,iface);
        break;
        case ARP_REPLY:
            Debug("ROUTER: ARP reply - update ARP table\n");
            sr_arp_set(sr, a_hdr->ar_sip, a_hdr->ar_sha, iface);
        break;
        default:
            Debug("ROUTER: ARP ERROR: don't know what %d is!\n", a_hdr->ar_op);
        }
    break;
    default:
        Debug("ROUTER: ERROR: don't know what %d ethernet packet type is!\n", e_hdr->ether_type);
    }

}/* end sr_handlepacket */

/**--------------------------------------------------------------------- 
 * Method: sr_router_send
 *
 * figure out where we are sending and do basic sanity check
 * this is where you'd buffer packets should you not be able to send them
 *---------------------------------------------------------------------*/
int sr_router_send(struct sr_ip_handle* h) 
{
        struct sr_arp*  arp_entry;
        struct sr_rt*   sender;
        struct sr_ethernet_hdr* eth;

        assert(h->sr);
        assert(h->pkt->ip.ip_dst.s_addr);

        sender = sr_rt_find(h->sr, h->pkt->ip.ip_dst.s_addr );
        arp_entry = sr_arp_get(h->sr, sender->gw.s_addr);

        if (arp_entry->tries > 5) {
                Debug("ROUTER: interface %s is disconnected (tries %d) - aborting\n", 
                        sender->interface, arp_entry->tries);
                return 0;
        } else if (arp_entry->tries > 0) {
                Debug("ROUTER: interface %s arp entry being refreshed (tries %d) buffering packet\n",
                        sender->interface, arp_entry->tries);
                sr_buffer_add(h);
                return 0;
        }
        Debug("ROUTER: attempting to send packet (size %d bytes) on interface %s\n", 
                h->len, sender->interface);

        /* set the mac addresses for the ethernet transmission based on our routing and arp data */
        eth = &h->pkt->eth;
        memcpy(
                eth->ether_shost,
                arp_entry->iface->addr,
                ETHER_ADDR_LEN
        );
        memcpy(
                eth->ether_dhost,
                arp_entry->mac,
                ETHER_ADDR_LEN
        );
        Debug("ROUTER: Source IP %s (send mac ", inet_ntoa(h->pkt->ip.ip_src));
        DebugMAC(eth->ether_shost); 
        Debug(") Destination IP %s (recv mac ", inet_ntoa(h->pkt->ip.ip_dst));
        DebugMAC(eth->ether_dhost);
        Debug(")\n");
        sr_send_packet(h->sr, h->raw, h->len, sender->interface);
        return 1;
}

/**
 * go through our queue and resend the packets 
 * delete anything too old or successfully sent
 */
void sr_router_resend(struct sr_instance* sr) {
        struct sr_buffer* b;
        struct sr_buffer_item *i, *next;
        struct ip* ip;
        time_t t;

        assert(sr);
        b = &sr->buffer;

        if (b->start) {
                i = b->start;
                while (i) {
                        ip = &i->h.pkt->ip;
                        Debug("ROUTER: attempting to resend packet (proto %d, from %s, ",
                                ip->ip_p, inet_ntoa(ip->ip_src));
                        Debug("to %s)\n", inet_ntoa(ip->ip_dst));
			next = i->next;
                        if (time(&t) - i->created > PACKET_TOO_OLD) { 
                                Debug("ROUTER: packet too old - deleting\n");
                                sr_buffer_remove(sr,i);
                        } else if (sr_router_send(&i->h)) {
                                Debug("ROUTER: packet successfully sent - deleting\n");
                                sr_buffer_remove(sr,i);
                        }
			i = next;
                }
        }
}

