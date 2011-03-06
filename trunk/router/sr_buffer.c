/**
 * @author Cal Woodruff <cwoodruf@sfu.ca>
 * shared packet buffer for router
 */
#include "sr_router.h"
#include "sr_buffer.h"
/**
 * initialize the buffer for the interface
 */
void sr_buffer_clear(struct sr_instance* sr) 
{
	struct sr_buffer* b;
	struct sr_buffer_item* i;
	assert(sr);
	b = &sr->buffer;
	while (b->start) {
		i = b->start;
		b->start = b->next;
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

	assert(h);

	sr = h->sr;
	assert(sr);
	b = &sr->buffer;

	i = (struct sr_buffer_item*)(malloc(sizeof(struct sr_buffer_item)));
	assert(i);
	i->h = *h;
	time(&i->created);
	i->next = 0;

	/* we are only item in list */
	if (!b->start)  {
		b->start = i;

	/* there are other items in the list */
	} else {
		b->end->next = i;
	}
	/* increment end of list */
	b->end = i;
}

/**
 * get a packet from an interface
 * @return the sr_buffer_item record
 */
struct sr_buffer_item* sr_buffer_getnext(struct sr_instance* sr) 
{
	assert(sr);
	return sr->buffer->start;
}
/** 
 * remove a buffer item from an interface's buffer
 */
void sr_buffer_removenext(struct sr_instance* sr) 
{
	struct sr_buffer_item* i;
	assert(sr);
	b = &sr->buffer;
	if (b->start) {
		i = b->start;
		/* not only item in list */
		if (b->start->next) {
			b->start = b->start->next;

		/* only item in list */
		} else {
			b->end = b->start = NULL;
		}
		free(i);
	}
}

