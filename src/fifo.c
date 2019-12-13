#include "fifo.h"

#define FLAGS_OVERRUN 0x0001

void fifo8_init(FIFO8 *fifo, int size, unsigned char *buf)
{
	fifo->buf = buf;
	fifo->size = size;
	fifo->free = size;
	fifo->flags = 0;
	fifo->next_r = 0;		
	fifo->next_w = 0;
}

int fifo8_put(FIFO8 *fifo, unsigned char data)
{
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->next_w] = data;
	fifo->next_w++;
	fifo->free--;
	if (fifo->next_w == fifo->size)
		fifo->next_w = 0;
	return 0;
}

int fifo8_get(FIFO8 *fifo)
{
	if (fifo->free == fifo->size)
		return -1;
	
	int data;
	data = fifo->buf[fifo->next_r];
	fifo->next_r++;
	if (fifo->next_r == fifo->size)
		fifo->next_r = 0;
	fifo->free++;

	return data;
}

int fifo8_status(FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}
