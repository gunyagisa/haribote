#pragma once

#include "fifo.h"

#define KEYCMD_SENDTO_MOUSE	0xd4
#define MOUSECMD_ENABLE		0xf4

typedef struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
} MOUSE_DEC;

void enable_mouse(FIFO32 *fifo, int data0, MOUSE_DEC *mdec);
int mouse_decode(MOUSE_DEC *mdec, unsigned char dat);
void inthandler2c(int *esp);
