/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 * shared buffer data structure used by interfaces
 */

#ifndef SR_BUFFER_H
#define SR_BUFFER_H

/**
 * data structure to pass into the sr_ip.c functions
 */
struct sr_ip_handle {
    struct sr_instance*         sr;
    uint8_t*                    raw;
    struct sr_ip_packet*        pkt;
    unsigned int                len;
    struct sr_if*               iface;
};

struct sr_buffer_item 
{
	struct sr_ip_handle h;
	time_t created;
	struct sr_buffer_item* next;
};
struct sr_buffer 
{
	struct sr_buffer_item* start;
	struct sr_buffer_item* end;
};

#endif
