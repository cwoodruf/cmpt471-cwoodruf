/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 * shared packet buffer for router
 */
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "sr_router.h"
#include "sr_buffer.h"
/**
 * initialize the buffer for the interface
 */
void sr_buffer_clear(struct sr_instance* sr) 
{
        struct sr_buffer_item* i,* b;
        assert(sr);
        b = sr->buffer.start;
        while (b) {
                i = b;
                b = b->next;
                free(i);        
        }
}
/** 
 * save a packet to the buffer 
 */
void sr_buffer_add(struct sr_ip_handle* h) 
{
        struct sr_instance* sr;
        struct sr_buffer* b;
        struct sr_buffer_item* i;
        struct ip* ip;
        uint8_t* raw;

        assert(h);
        if (h->buffered) return;

        sr = h->sr;
        assert(sr);
        b = &sr->buffer;

        raw = (uint8_t*) malloc(h->raw_len);
        if (!raw) {
                Debug("BUFFER: not enough memory to save packet - aborting!\n");
                return;
        }
        memcpy(raw, h->raw, h->raw_len);
        
        i = (struct sr_buffer_item*)(malloc(sizeof(struct sr_buffer_item)));
        if (!i) {
                Debug("BUFFER: out of memory for buffer item - aborting!\n");
                free(raw);
                return;
        }
        h->buffered = i;
        i->h = *h;
        i->h.raw = raw;
        i->h.pkt = (struct sr_ip_packet*)raw;
        time(&i->created);
        i->next = 0;

        ip = &i->h.pkt->ip;
        Debug("ROUTER: buffering packet (proto %d, from %s, to %s)\n",
                ip->ip_p, inet_ntoa(ip->ip_src), inet_ntoa(ip->ip_dst));
        /* we are only item in list */
        if (!b->start)  {
                b->start = i;
                i->prev = 0;

        /* there are other items in the list */
        } else {
                b->end->next = i;
                i->prev = b->end;
        }
        /* increment end of list */
        b->end = i;
}

/** 
 * remove a buffer item from an interface's buffer
 */
void sr_buffer_remove(struct sr_instance* sr, struct sr_buffer_item* item) 
{
        struct sr_buffer_item* prev,* next,* delitem;
        struct sr_buffer* b;

        assert(sr);
        b = &sr->buffer;

        if (item) {
                delitem = item;
                /* not only item in list */
                if (item->next || item->prev) {
                        if (b->end == item) b->end = item->prev;
                        if (b->start == item) b->start = item->next;
                        prev = item->prev;
                        next = item->next;
                        if (next) next->prev = prev;
                        if (prev) prev->next = next;

                /* only item in list */
                } else {
                        b->end = b->start = b->pos = 0;
                }
                free(delitem);
        }
}

