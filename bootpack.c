#include "bootpack.h"
#include "graphic.h"
#include "dsctbl.h"
#include "interrupt.h"
#include "mysprintf.c"
#include "fifo.h"

#define PORT_KEYCMD		0x0064
#define PORT_KEYDATA		0x0060
#define PORT_KEYSTA		0x0064
#define KEYCMD_WRITE_MODE	0x60
#define KBC_MODE_MOUSE		0x47

#define KEYCMD_SENDTO_MOUSE	0xd4
#define MOUSECMD_ENABLE		0xf4

extern void sprintf(char *str, char *fmt, ...);

void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & 0x02) == 0)
			break;
	}
}

void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA, KBC_MODE_MOUSE);
}

void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA, MOUSECMD_ENABLE);
}
	

void HariMain(void) {
	BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
	char mouse[256], s[40], keybuf[32];
	int mx, my, d;
	mx = 100;
	my = 100;

	fifo8_init(&keyfifo, 32, keybuf);

	init_keyboard();
	enable_mouse();

	init_gdtidt();
	init_pic();
	io_sti();

	init_palette();	//configure color setting
	init_screen(binfo);
	init_mouse_cursor8(mouse, COL8_008484);
	block_renderer8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mouse, 16);

	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);

	d = 0;
	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) == 0) {
			io_stihlt();
		} else {
			d = fifo8_get(&keyfifo);
			io_sti();
			sprintf(s, "%x", d);
			boxfill8(binfo, COL8_008484, 0, 16, 15, 31);
			str_renderer8(binfo, COL8_FFFFFF, 0, 16, s); 
		}
	}
}

