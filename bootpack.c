#include "bootpack.h"
#include "graphic.h" 
#include "dsctbl.h"
#include "interrupt.h"
#include "fifo.h"
#include "mysprintf.c"

#define PORT_KEYCMD		0x0064
#define PORT_KEYDATA		0x0060
#define PORT_KEYSTA		0x0064
#define KEYCMD_WRITE_MODE	0x60
#define KBC_MODE_MOUSE		0x47

#define KEYCMD_SENDTO_MOUSE	0xd4
#define MOUSECMD_ENABLE		0xf4

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

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

void enable_mouse(struct MOUSE_DEC *mdec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA, MOUSECMD_ENABLE);
    mdec->phase = 0;
}
	
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
    if (mdec->phase == 0) {
        if (dat == 0xfa)
            mdec->phase = 1;
        return 0;
    }
    if (mdec->phase == 1) {
        if ((dat & 0xc8) == 0x08) {
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if (mdec->phase == 2) {
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if (mdec->phase == 3) {
        mdec->buf[2] = dat;
        mdec->phase = 1;
        // decode
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if ((mdec->buf[0] & 0x10) != 0)
            mdec->x |= 0xffffff00;
        if ((mdec->buf[0] & 0x20) != 0)
            mdec->y |= 0xffffff00;
        mdec->y = - mdec->y;
        return 1;
    }
    return -1;
}

void HariMain(void) {
	BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
	char *mouse, s[40], keybuf[32], mousebuf[128];
	int mx, my, d;
    struct MOUSE_DEC mdec;
	mx = 100;
	my = 100;
	
	init_gdtidt();
	init_pic();
	io_sti();
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);

	init_keyboard();

	init_palette();	//configure color setting
	init_screen(binfo);
	init_mouse_cursor8(mouse, COL8_008484);
	block_renderer8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mouse, 16);

	enable_mouse(&mdec);
	
	for (;;) {
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();
        } else {
            if (fifo8_status(&mousefifo) != 0) {
                d = fifo8_get(&mousefifo);
                io_sti();
                if (mouse_decode(&mdec, d) != 0) {
                    sprintf(s, "[lcr %d   %d  ]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0)
                        s[1] = 'L';
                    if ((mdec.btn & 0x02) != 0)
                        s[3] = 'R';
                    if ((mdec.btn & 0x04) != 0)
                        s[2] = 'C';
                    boxfill8(binfo, COL8_008484, 32, 16, 32 + 8 * 15 - 1, 31);
                    str_renderer8(binfo, COL8_FFFFFF, 32, 16, s);


                    //mouse
                    boxfill8(binfo, COL8_008484, mx, my, mx+15, my+15);
                    mx += mdec.x;
                    my += mdec.y;

                    if (mx < 0)
                        mx = 0;
                    if (my < 0)
                        my = 0;
                    if (mx > binfo->scrnx - 16)
                        mx = binfo->scrnx - 16;
                    if (my > binfo->scrny - 16)
                        my = binfo->scrny - 16;

                    sprintf(s, "(%d   , %d)", my, my);
                    boxfill8(binfo, COL8_008484, 0, 0, 79, 15);
                    str_renderer8(binfo, COL8_FFFFFF, 0, 0, s);
                    block_renderer8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mouse, 16);
                }
            } else if (fifo8_status(&keyfifo) != 0) {
                d = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "%x", d);
                boxfill8(binfo, COL8_008484, 0, 16, 15, 31);
                str_renderer8(binfo, COL8_FFFFFF, 0, 16, s); 
            }
        }
	}
}

