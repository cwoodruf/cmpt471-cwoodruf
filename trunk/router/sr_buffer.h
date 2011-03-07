/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 * shared buffer data structure used by interfaces
 */

#ifndef SR_BUFFER_H
#define SR_BUFFER_H

/** hold packets for this many seconds if they are buffered */
#define PACKET_TOO_OLD 600

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
};

struct sr_buffer 
{
        struct sr_buffer_item* start;
        struct sr_buffer_item* pos;
        struct sr_buffer_item* end;
};

#endif
