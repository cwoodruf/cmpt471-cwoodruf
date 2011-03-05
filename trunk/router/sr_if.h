/*-----------------------------------------------------------------------------
 * file:  sr_if.h
 * date:  Sun Oct 06 14:13:13 PDT 2002 
 * Contact: casado@stanford.edu 
 *
 * Description:
 *
 * Data structures and methods for handeling interfaces
 *
 *---------------------------------------------------------------------------*/

#ifndef sr_INTERFACE_H
#define sr_INTERFACE_H

#ifdef _LINUX_
#include <stdint.h>
#endif /* _LINUX_ */

#ifdef _SOLARIS_
#include </usr/include/sys/int_types.h>
#endif /* SOLARIS */

#ifdef _DARWIN_
#include <inttypes.h>
#endif

#define sr_IFACE_NAMELEN 32
#define IFACE_MAX 256

#include "vnscommand.h"
#include "sr_protocol.h"
#include "sr_ip.h"

struct sr_instance;

/* ----------------------------------------------------------------------------
 * struct sr_if
 *
 * Node in the interface list for each router
 *
 * -------------------------------------------------------------------------- */

/** deliberately allocate all the buffers we need up front */
#define SR_BUFF_SIZE 256
struct sr_packet {
	struct sr_ip_packet pkt;
	uint8_t arp_entry;
	time_t  created;
};

struct sr_if
{
    char name[sr_IFACE_NAMELEN];
    unsigned char addr[ETHER_ADDR_LEN];
    uint32_t ip;
    uint32_t speed;
    /** store the queue of unsent packets with the interface */
    struct sr_packet sendqueue[SR_BUFF_SIZE];
    /** which packet to send next in sendqueue */
    uint16_t sendnext;
    struct sr_if* next;
};

struct sr_if* sr_get_interface(struct sr_instance* sr, const char* name);
void sr_add_interface(struct sr_instance*, const char*);
void sr_set_ether_addr(struct sr_instance*, const unsigned char*);
void sr_set_ether_ip(struct sr_instance*, uint32_t ip_nbo);
void sr_print_if_list(struct sr_instance*);
void sr_print_if(struct sr_if*);

#endif /* --  sr_INTERFACE_H -- */
