#include "bootpack.h"
#include "graphic.h"
#include "dsctbl.h"
#include "interrupt.h"
#include "mysprintf.c"
#include "fifo.c"

extern void sprintf(char *str, char *fmt, ...);

void HariMain(void) {
	BOOTINFO *binfo = (BOOTINFO *) ADDR_BOOTINFO;
	char mouse[256], s[40], keybuf[32];
	int mx, my, d;
	mx = 100;
	my = 100;

	fifo8_init(&keyfifo, 32, keybuf);

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
			str_renderer8(binfo, 0, 16, COL8_FFFFFF, s); 
		}
	}
}

