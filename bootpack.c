#include "bootpack.h"
#include "graphic.h" 
#include "dsctbl.h"
#include "interrupt.h"
#include "fifo.h"
#include "mouse.h"
#include "keyboard.h"
#include "mysprintf.c"

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
