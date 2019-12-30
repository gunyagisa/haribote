#pragma once

typedef struct FIFO32 {
	int *buf;
	int next_r, next_w, size, free, flags;
} FIFO32;

void fifo32_init(FIFO32 *fifo, int size, int *buf);
int fifo32_put(FIFO32 *fifo, int data);
int fifo32_get(FIFO32 *fifo);
int fifo32_status(FIFO32 *fifo);
void inthandler21(int *esp);

