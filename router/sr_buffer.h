/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 * shared buffer data structure used by interfaces
 */

#ifndef SR_BUFFER_H
#define SR_BUFFER_H

#include "vnscommand.h"

/** hold packets for this many seconds if they are buffered */
#define PACKET_TOO_OLD 6
/** how many packets to save - this is about 5x what is seen in test */
#define BUFFSIZE 512

/**
 * data structure to pass into the sr_ip.c functions
 */
struct sr_buffer_item;
struct sr_ip_handle {
    struct sr_instance*         sr;
    uint8_t*                    raw;
    unsigned int                raw_len;
    struct sr_ip_packet*        pkt;
    unsigned int                len;
    struct sr_buffer_item*      buffered;
    struct sr_if*               iface;
};

struct sr_buffer_item 
{
        struct sr_ip_handle h;
        time_t created;
        struct sr_buffer_item* prev;
        struct sr_buffer_item* next;
	int    pos;
};

struct sr_buffer 
{
	struct sr_buffer_item items[BUFFSIZE];
	uint8_t packets[BUFFSIZE][VNSCMDSIZE+MPADDING];
        struct sr_buffer_item* start;
        struct sr_buffer_item* end;
};

#endif
