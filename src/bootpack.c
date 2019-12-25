#include "bootpack.h"
#include "graphic.h" 
#include "dsctbl.h"
#include "interrupt.h"
#include "fifo.h"
#include "mouse.h"
#include "keyboard.h"
#include "mysprintf.h"
#include "memory.h"
#include "sheet.h"


void HariMain(void) 
{
    BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
    char s[40], keybuf[32], mousebuf[128];
    int mx, my, d;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse;
    unsigned char *buf_back, buf_mouse[256];

    unsigned int memtotal;
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_gdtidt();
    init_pic();
    io_sti();
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    init_keyboard();

    init_palette();	//configure color setting
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(buf_mouse, COL8_008484);
    sheet_slide(shtctl, sht_back, 0, 0);
    sheet_slide(shtctl, sht_mouse, mx, my);
    sheet_updown(shtctl, sht_back, 0);
    sheet_updown(shtctl, sht_mouse, 1);
    sprintf(s, "(%d   , %d)", mx, my);
    str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, 0, s);
    sprintf(s, "Memory Size: %dMB  free: %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, 32, s);
    sheet_refresh(shtctl);

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
                    boxfill8(buf_back,binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 15 - 1, 31);
                    str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 32, 16, s);


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

                    sprintf(s, "(%d   , %d)", mx, my);
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, 0, s);
                    sheet_slide(shtctl, sht_mouse, mx, my);
                }
            } else if (fifo8_status(&keyfifo) != 0) {
                d = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "%x", d);
                boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, 16, s); 
                sheet_refresh(shtctl);
            }
        }
    }
}
