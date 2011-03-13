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
 * our versions of malloc and free that 
 * use a fixed array to avoid memory corruption problems
 */
struct sr_buffer_item* sr_buffer_malloc(struct sr_instance* sr) 
{
        struct sr_buffer_item* b;
        int i;

        assert(sr);

        for (i=0; i<BUFFSIZE; i++) {
                b = &sr->buffer.items[i];
                if (b->h.buffered == 0) {
                        b->h.raw = sr->buffer.packets[i];
                        b->h.buffered = b;
                        b->pos = i;
                        return b;
                }
        }
        return NULL;
}
void sr_buffer_free(struct sr_instance* sr, struct sr_buffer_item* item) 
{
        uint8_t* s;

        assert(sr);
        if (!item->h.buffered) return;
        if (item->pos < 0 || item->pos >= BUFFSIZE) return;

        s = sr->buffer.packets[item->pos];

        memset(s,0,sizeof(*s));
        item->h.buffered = 0;
}
/**
 * initialize the buffer for the interface
 */
void sr_buffer_clear(struct sr_instance* sr) 
{
        assert(sr);

        memset(&sr->buffer,0,sizeof(struct sr_buffer));

        sr->buffer.start = sr->buffer.end = 0;
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

        assert(h);
        if (h->buffered) return;

        sr = h->sr;
        assert(sr);
        b = &sr->buffer;

        i = sr_buffer_malloc(sr);
        if (!i) {
                Debug("BUFFER: out of memory for buffer item - aborting!\n");
                return;
        }
        h->buffered = i;
        i->h = *h;
        memcpy(i->h.raw, h->raw, h->raw_len);
        i->h.pkt = (struct sr_ip_packet*)i->h.raw;
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
                        b->end = b->start = 0;
                }
                sr_buffer_free(sr,delitem);
        }
}

