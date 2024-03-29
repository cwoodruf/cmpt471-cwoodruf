/*-----------------------------------------------------------------------------
 * file:  sr_inface.
 * date:  Sun Oct 06 14:13:13 PDT 2002 
 * Contact: casado@stanford.edu 
 *
 * Description:
 *
 * Data structures and methods for handling interfaces
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifdef _DARWIN_
#include <sys/types.h>
#endif /* _DARWIN_ */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sr_if.h"
#include "sr_router.h"

/**
 * get an index into the routing array based on the last characters of the name 
 * assumes a name like "eth0" or "eth12"
 */
uint8_t sr_if_name2idx(const char* name) 
{
        uint16_t idx = atoi(name+3);

        assert(idx < LAN_SIZE);

        return (uint8_t) idx;
}
/**
 * unfortunately the vns server itself sends these "eth" strings for the interfaces
 * when we process a packet so we have to do this look up every packet
 */ 
struct sr_if* sr_if_name2iface(struct sr_instance* sr, const char* name) 
{
        return sr->interfaces[ sr_if_name2idx(name) ];
}

/**
 * find an interface from its ip address
 * check to make sure it really is for us by checking the ip address
 */
struct sr_if* sr_if_ip2iface(struct sr_instance* sr, uint32_t ip) 
{
        struct sr_if* i;
        assert(sr);
        assert(ip);
        i = sr->ip2iface[ (ntohl(ip) & 0xFF) ];
        if (i) {
                if (i->ip == ip) return i;
        }
        return NULL;
}

/**
 * clear all iface related variables
 */
void sr_if_clear(struct sr_instance* sr) {
        int i;
        assert(sr);
        for (i=0; i<=LAN_SIZE; i++) {
                if (sr->interfaces[i]) {
                        free(sr->interfaces[i]);
                        sr->interfaces[i] = 0;
                }
                if (sr->ip2iface[i]) {
                        free(sr->ip2iface[i]);
                        sr->ip2iface[i] = 0;
                }
        }
        sr->if_list = 0;
}
/*--------------------------------------------------------------------- 
 * Method: sr_get_interface
 * Scope: Global
 *
 * Given an interface name return the interface record or 0 if it doesn't
 * exist.
 *
 *---------------------------------------------------------------------*/

struct sr_if* sr_get_interface(struct sr_instance* sr, const char* name)
{
    struct sr_if* if_walker = 0;

    /* -- REQUIRES -- */
    assert(name);
    assert(sr);

    if_walker = sr->if_list;

    while(if_walker)
    {
       if(!strncmp(if_walker->name,name,sr_IFACE_NAMELEN))
        { return if_walker; }
        if_walker = if_walker->next;
    }

    return 0;
} /* -- sr_get_interface -- */

/*--------------------------------------------------------------------- 
 * Method: sr_add_interface(..)
 * Scope: Global
 *
 * Add and interface to the router's list
 *
 *---------------------------------------------------------------------*/
void sr_add_interface(struct sr_instance* sr, const char* name)
{
    struct sr_if* if_walker = 0;
    int i;

    /* -- REQUIRES -- */
    assert(name);
    assert(sr);

    i = sr_if_name2idx(name);
    assert(i >= 0);

    /* -- empty list special case -- */
    if(sr->if_list == 0)
    {
        sr->interfaces[i] = sr->if_list = (struct sr_if*)malloc(sizeof(struct sr_if));
        assert(sr->if_list);
        sr->if_list->next = 0;
        strncpy(sr->if_list->name,name,sr_IFACE_NAMELEN);
        return;
    }

    /* -- find the end of the list -- */
    if_walker = sr->if_list;
    while(if_walker->next)
    {if_walker = if_walker->next; }
    
    /* we should not overwrite an existing interface */
    assert(sr->interfaces[i] == 0);

    sr->interfaces[i] = if_walker->next = (struct sr_if*)malloc(sizeof(struct sr_if));

    assert(if_walker->next);
    if_walker = if_walker->next;
    strncpy(if_walker->name,name,sr_IFACE_NAMELEN);
    if_walker->next = 0;
} /* -- sr_add_interface -- */ 

/*--------------------------------------------------------------------- 
 * Method: sr_sat_ether_addr(..)
 * Scope: Global
 *
 * set the ethernet address of the LAST interface in the interface list 
 *
 *---------------------------------------------------------------------*/

void sr_set_ether_addr(struct sr_instance* sr, const unsigned char* addr)
{
    struct sr_if* if_walker = 0;

    /* -- REQUIRES -- */
    assert(sr->if_list);
    
    if_walker = sr->if_list;
    while(if_walker->next)
    {if_walker = if_walker->next; }

    /* -- copy address -- */
    memcpy(if_walker->addr,addr,6);

} /* -- sr_set_ether_addr -- */

/*--------------------------------------------------------------------- 
 * Method: sr_set_ether_ip(..)
 * Scope: Global
 *
 * set the IP address of the LAST interface in the interface list
 *
 *---------------------------------------------------------------------*/

void sr_set_ether_ip(struct sr_instance* sr, uint32_t ip_nbo)
{
    struct sr_if* if_walker = 0;

    /* -- REQUIRES -- */
    assert(sr->if_list);
    
    if_walker = sr->if_list;
    while(if_walker->next)
    {if_walker = if_walker->next; }

    /* -- copy address -- */
    if_walker->ip = ip_nbo;

    /** tie the ip to the interface record directly so we don't have to search */
    sr->ip2iface[ (ntohl(ip_nbo) & 0xFF) ] = if_walker;

} /* -- sr_set_ether_ip -- */

/*--------------------------------------------------------------------- 
 * Method: sr_print_if_list(..)
 * Scope: Global
 *
 * print out the list of interfaces to stdout
 *
 *---------------------------------------------------------------------*/

void sr_print_if_list(struct sr_instance* sr)
{
    struct sr_if* if_walker = 0;
    uint16_t i;

    for (i=0; i<LAN_SIZE; i++) {
        if (sr->interfaces[ i ] != NULL) {
            printf("IF: interfaces[%d]:\n", i);
            sr_print_if(sr->interfaces[ i ]);
        }
        if (sr->ip2iface[ i ] != NULL) {
            printf("IF: ip2iface[%d]:\n", i);
            sr_print_if(sr->ip2iface[ i ]);
        }
    }
    printf("\n");

    if(sr->if_list == 0)
    {
        printf(" Interface list empty \n");
        return;
    }

    if_walker = sr->if_list;
    
    sr_print_if(if_walker);
    while(if_walker->next)
    {
        if_walker = if_walker->next; 
        sr_print_if(if_walker);
    }

} /* -- sr_print_if_list -- */

/*--------------------------------------------------------------------- 
 * Method: sr_print_if(..)
 * Scope: Global
 *
 * print out a single interface to stdout
 *
 *---------------------------------------------------------------------*/

void sr_print_if(struct sr_if* iface)
{
    struct in_addr ip_addr;

    /* -- REQUIRES --*/
    assert(iface);
    assert(iface->name);

    ip_addr.s_addr = iface->ip;

    Debug("%s\tHWaddr ",iface->name);
    DebugMAC(iface->addr);
    Debug("\n");
    Debug("\tinet addr %s\n",inet_ntoa(ip_addr));
} /* -- sr_print_if -- */
