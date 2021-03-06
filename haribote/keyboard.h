#pragma once

#include "fifo.h"

#define PORT_KEYCMD		0x0064
#define PORT_KEYDATA		0x0060
#define PORT_KEYSTA		0x0064
#define KEYCMD_WRITE_MODE	0x60
#define KBC_MODE_MOUSE		0x47

void wait_KBC_sendready(void);
void init_keyboard(FIFO32 *fifo, int keydata0);
