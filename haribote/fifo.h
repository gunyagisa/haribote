#pragma once

typedef struct FIFO32 {
  int *buf;
  int next_r, next_w, size, free, flags;
  struct TASK *task;
} FIFO32;

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);
void inthandler21(int *esp);

