#include "graphic.h"

void init_palette(void) {
	static unsigned char table_rgb[16 * 3] = { //DB instruction
		0x00, 0x00, 0x00,
		0xff, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0xff, 0xff, 0x00,
		0x00, 0x00, 0xff,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0xff,
		0xff, 0xff, 0xff,
		0xc6, 0xc6, 0xc6,
		0x84, 0x00, 0x00,
		0x00, 0x84, 0x00,
		0x84, 0x84, 0x84,
		0x00, 0x00, 0x84,
		0x84, 0x00, 0x84,
		0x00, 0x84, 0x84,
		0x84, 0x84, 0x84,
	};

	set_palette(0, 15, table_rgb);
}

void set_palette(int start, int end, unsigned char *rgb) {
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x3c8, start);
	for (i = start;i <= end;i++) {
		io_out8(0x3c9, rgb[0] / 4);
		io_out8(0x3c9, rgb[1] / 4);
		io_out8(0x3c9, rgb[2] / 4);
		rgb += 3;
	}

	io_store_eflags(eflags);
}

void init_screen(unsigned char *vram, int xsize, int ysize)
{

	boxfill8(vram, xsize, COL8_008484,	0, 			0, xsize - 1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6, 	0, ysize - 28, xsize - 1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF, 	0, ysize - 27, xsize - 1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6, 	0, ysize - 26, xsize - 1, ysize - 1);

	boxfill8(vram, xsize, COL8_FFFFFF, 	3, ysize - 24, 59, ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF, 	2, ysize - 24, 2, ysize - 4);
	boxfill8(vram, xsize, COL8_848484, 	3,	ysize - 4, 59, ysize - 4);
	boxfill8(vram, xsize, COL8_848484,59,	ysize - 23, 59, ysize - 5);
	boxfill8(vram, xsize, COL8_000000, 	2, ysize - 3, 59, ysize - 3);
	boxfill8(vram, xsize, COL8_000000, 	60, ysize -24, 60, ysize -3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize -47, ysize - 23, xsize - 47, ysize - 4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
}	


void boxfill8(unsigned char *vram, int bx, unsigned char color_code, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0;x <= x1;x++) {
			vram[x + y * bx] = color_code;
		}
	}
}

void font_renderer8(unsigned char *vram, int xsize, char color_code, int x, int y,  char *font) 
{
	int i, j;
	char *p, d;
	
	for (i = 0;i < 16;i++) {
		p =(char *) (vram + x + (y + i) * xsize);
		d = font[i];
		for (j = 0;j < 8;j++) { 
			if ((d & (0x01 << 7 - j)) != 0) {
				p[j] = color_code;
			}
		}
	}
}

void str_renderer8(unsigned char *buf, int xsize, char color_code, int x, int y, unsigned char *str)
{
	extern char hankaku[4096];
	for (;*str != 0x00;str++) {
		font_renderer8(buf, xsize, color_code, x, y, hankaku + *str * 16);
		x += 8;
	}
}

void init_mouse_cursor8(char *mouse, char back_color)
{
	static char cursor[16][16] = {
		"**************..",
		"*00000000000*...",
		"*0000000000*....",
		"*000000000*.....",
		"*00000000*......",
		"*0000000*.......",
		"*0000000*.......",
		"*00000000*......",
		"*0000**000*.....",
		"*000*..*000*....",
		"*00*....*000*...",
		"*0*......*000*..",
		"**........*000*.",
		"*..........*000*",
		"............*00*",
		".............***",
	};

	int x, y;
	
	for (y = 0;y < 16;y++) {
		for (x = 0;x < 16;x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			} else if (cursor[y][x] == '0') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			} else if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = back_color;
			}
		}
	}
}

void block_renderer8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	
	for (y = 0;y < pysize;y++) {
		for (x = 0;x < pxsize;x++) {
			vram[(px0 + x) + (py0 + y) * vxsize] = buf[y * bxsize + x];
		}
	}
}
