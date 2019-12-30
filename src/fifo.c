#include "fifo.h"

#define FLAGS_OVERRUN 0x0001

void fifo32_init(FIFO32 *fifo, int size, int *buf)
{
	fifo->buf = buf;
	fifo->size = size;
	fifo->free = size;
	fifo->flags = 0;
	fifo->next_r = 0;		
	fifo->next_w = 0;
}

int fifo32_put(FIFO32 *fifo, int data)
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

int fifo32_get(FIFO32 *fifo)
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

int fifo32_status(FIFO32 *fifo)
{
	return fifo->size - fifo->free;
}
