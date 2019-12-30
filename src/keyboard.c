#include "bootpack.h"
#include "keyboard.h"
#include "interrupt.h"
#include "fifo.h"

FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void)
{
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & 0x02) == 0)
            break;
    }
}

void init_keyboard(FIFO32 *fifo, int data0)
{
    keyfifo = fifo;
    keydata0 = data0;

    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDATA, KBC_MODE_MOUSE);
}

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(KEYDATA_PORT);
	fifo32_put(keyfifo, data + keydata0);
}
