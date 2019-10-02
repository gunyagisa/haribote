#include "bootpack.h"
#include "graphic.h"
#include "dsctbl.h"
#include "interrupt.h"
#include "mysprintf.c"

extern void sprintf(char *str, char *fmt, ...);

void HariMain(void) {
	BOOTINFO *binfo = (BOOTINFO *) ADDR_BOOTINFO;
	char *mouse, *s;
	int mx, my, d;
	mx = 100;
	my = 100;


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
		if (keybuf.len == 0) {
			io_stihlt();
		} else {
			d = keybuf.data[keybuf.next_r];
			keybuf.next_r++;
			keybuf.len--;
			if (keybuf.next_r == 32)
				keybuf.next_r = 0;
			io_sti();
			sprintf(s, "%x", d);
			boxfill8(binfo, COL8_008484, 0, 16, 15, 31);
			str_renderer8(binfo, 0, 16, COL8_FFFFFF, s); 
		}
	}
}

