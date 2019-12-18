#pragma once

typedef struct FIFO8 {
	unsigned char *buf;
	int next_r, next_w, size, free, flags;
} FIFO8;

void fifo8_init(FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(FIFO8 *fifo, unsigned char data);
int fifo8_get(FIFO8 *fifo);
int fifo8_status(FIFO8 *fifo);

