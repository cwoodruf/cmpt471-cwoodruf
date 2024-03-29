/*-----------------------------------------------------------------------------
 * file:  sr_rt.c
 * date:  Mon Oct 07 04:02:12 PDT 2002  
 * Author:  casado@stanford.edu
 *
 * Description:
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>


#include <sys/socket.h>
#include <netinet/in.h>
#define __USE_MISC 1 /* force linux to show inet_aton */
#include <arpa/inet.h>

#include "sr_rt.h"
#include "sr_router.h"

/*--------------------------------------------------------------------- 
 * Method: sr_rt_find
 *
 * author Cal Woodruff <cwoodruf@sfu.ca>
 *
 * find the correct routing table entry for the ip address
 * this is painful as we have to scan the whole table every time
 *
 * returns address of rt entry
 *---------------------------------------------------------------------*/
struct sr_rt* sr_rt_find(struct sr_instance* sr, uint32_t ip) 
{
        struct sr_rt* walker,* elsewhere,* bestmatch;
        uint32_t prefix, bestprefix;

        assert(sr);
        assert(ip);

        walker = sr->routing_table;
        assert(walker);

        prefix = bestprefix = 0;

        if (walker->dest.s_addr == 0) {
                elsewhere = walker;
        } else if (
                (prefix = (walker->dest.s_addr & walker->mask.s_addr)) == 
                        (ip & walker->mask.s_addr)
        ) {
                bestmatch = walker;
                bestprefix = prefix;
                if (walker->mask.s_addr == 0xFFFFFFFF) return walker;
        }

        while (walker->next) {
                walker = walker->next;
                if (walker->dest.s_addr == 0) {
                        elsewhere = walker;
                } else if (
                        (prefix = (walker->dest.s_addr & walker->mask.s_addr)) == 
                                (ip & walker->mask.s_addr)
                ) {
                        if (bestprefix == 0 || prefix > bestprefix) {
                                bestprefix = prefix;
                                bestmatch = walker;
                                if (walker->mask.s_addr == 0xFFFFFFFF) return walker;
                        }
                }
        }
        if (bestprefix == 0) return elsewhere;
        return bestmatch;
}
/**
 * free routing table 
 */
void sr_rt_clear(struct sr_instance* sr) {
        struct sr_rt *r, *del;

        assert(sr);
        r = sr->routing_table;
        while (r) {
                del = r;
                r = r->next;
                free(del);
        }
        sr->routing_table = 0;
}
/*--------------------------------------------------------------------- 
 * Method:
 *
 *---------------------------------------------------------------------*/

int sr_load_rt(struct sr_instance* sr,const char* filename)
{
    FILE* fp;
    char  line[BUFSIZ];
    char  dest[32];
    char  gw[32];
    char  mask[32];
    char  iface[32];
    struct in_addr dest_addr;
    struct in_addr gw_addr;
    struct in_addr mask_addr;

    /* -- REQUIRES -- */
    assert(filename);
    if( access(filename,R_OK) != 0)
    {
        perror("access");
        return -1;
    }

    fp = fopen(filename,"r");

    while( fgets(line,BUFSIZ,fp) != 0)
    {
        sscanf(line,"%s %s %s %s",dest,gw,mask,iface);
        if(inet_aton(dest,&dest_addr) == 0)
        { 
            fprintf(stderr,
                    "Error loading routing table, cannot convert %s to valid IP\n",
                    dest);
            return -1; 
        }
        if(inet_aton(gw,&gw_addr) == 0)
        { 
            fprintf(stderr,
                    "Error loading routing table, cannot convert %s to valid IP\n",
                    gw);
            return -1; 
        }
        if(inet_aton(mask,&mask_addr) == 0)
        { 
            fprintf(stderr,
                    "Error loading routing table, cannot convert %s to valid IP\n",
                    mask);
            return -1; 
        }
        sr_add_rt_entry(sr,dest_addr,gw_addr,mask_addr,iface);
    } /* -- while -- */

    return 0; /* -- success -- */
} /* -- sr_load_rt -- */

/*--------------------------------------------------------------------- 
 * Method:
 *
 *---------------------------------------------------------------------*/

void sr_add_rt_entry(struct sr_instance* sr, struct in_addr dest,
        struct in_addr gw, struct in_addr mask,char* if_name)
{
    struct sr_rt* rt_walker = 0;

    /* -- REQUIRES -- */
    assert(if_name);
    assert(sr);

    /* -- empty list special case -- */
    if(sr->routing_table == 0)
    {
        sr->routing_table = (struct sr_rt*)malloc(sizeof(struct sr_rt));
        assert(sr->routing_table);
        sr->routing_table->next = 0;
        sr->routing_table->dest = dest;
        sr->routing_table->gw   = gw;
        sr->routing_table->mask = mask;
        sr->routing_table->ifidx = sr_if_name2idx(if_name);
        strncpy(sr->routing_table->interface,if_name,sr_IFACE_NAMELEN);
        return;
    }

    /* -- find the end of the list -- */
    rt_walker = sr->routing_table;
    while(rt_walker->next)
    {rt_walker = rt_walker->next; }

    rt_walker->next = (struct sr_rt*)malloc(sizeof(struct sr_rt));
    assert(rt_walker->next);
    rt_walker = rt_walker->next;

    rt_walker->next = 0;
    rt_walker->dest = dest;
    rt_walker->gw   = gw;
    rt_walker->mask = mask;
    strncpy(rt_walker->interface,if_name,sr_IFACE_NAMELEN);

} /* -- sr_add_entry -- */

/*--------------------------------------------------------------------- 
 * Method:
 *
 *---------------------------------------------------------------------*/

void sr_print_routing_table(struct sr_instance* sr)
{
    struct sr_rt* rt_walker = 0;

    if(sr->routing_table == 0)
    {
        printf("RT:  *warning* Routing table empty \n");
        return;
    }

    printf("RT: %-20s %-20s %-20s %-20s\n","Destination","Gateway","Mask","Iface");

    rt_walker = sr->routing_table;
    
    sr_print_routing_entry(rt_walker);
    while(rt_walker->next)
    {
        rt_walker = rt_walker->next; 
        sr_print_routing_entry(rt_walker);
    }

} /* -- sr_print_routing_table -- */

/*--------------------------------------------------------------------- 
 * Method:
 *
 *---------------------------------------------------------------------*/

void sr_print_routing_entry(struct sr_rt* entry)
{
    /* -- REQUIRES --*/
    assert(entry);
    assert(entry->interface);

    printf("RT: %-20s ",inet_ntoa(entry->dest));
    printf("%-20s ",inet_ntoa(entry->gw));
    printf("%-20s ",inet_ntoa(entry->mask));
    printf("%-20s\n",entry->interface);

} /* -- sr_print_routing_entry -- */
