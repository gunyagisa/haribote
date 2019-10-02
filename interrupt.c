#include "interrupt.h"
#include "graphic.h"
#include "bootpack.h"
#include "fifo.h"

void init_pic(void)
{
	io_out8(PIC0_IMR, 0xff);
	io_out8(PIC1_IMR, 0xff);

	io_out8(PIC0_ICW1,0x11);
	io_out8(PIC0_ICW2, 0x20);
	io_out8(PIC0_ICW3, 1 << 2);
	io_out8(PIC0_ICW4, 0x01);

	io_out8(PIC1_ICW1, 0x11);
	io_out8(PIC1_ICW2, 0x28);
	io_out8(PIC1_ICW3, 2);
	io_out8(PIC1_ICW4, 0x01);

	io_out8(PIC0_IMR, 0xfb);
	io_out8(PIC1_IMR, 0xff);
}

KEYBUF keybuf;
FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(KEYDATA_PORT);
	fifo8_put(&keyfifo, data);
}

void inthandler2c(int *esp)
{
	BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR; 
	boxfill8(binfo, COL8_000000, 0, 0, 32*8 - 1, 15);
	str_renderer8(binfo, COL8_FFFFFF, 0, 0, "INT 2c (IRQ-12) PS/2 mouse");
}
