#include "bootpack.h"
#include "graphic.h" 
#include "dsctbl.h"
#include "interrupt.h"
#include "fifo.h"
#include "mouse.h"
#include "keyboard.h"
#include "mysprintf.h"

unsigned int memtest_sub(unsigned int start, unsigned int end);

unsigned int memtest(unsigned int start, unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i; 
    // check 386 or 486 later
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }

    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
    unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    for (i = start;i <= end;i += 0x1000) {
        p = (unsigned int *) (i + 0xffc);
        old = *p;
        *p = pat0;
        *p ^= 0xffffffff;
        if (*p != pat1) {
not_memory:
            *p = old;
            break;
        }

        *p ^= 0xffffffff;
        if (*p != pat0) {
            goto not_memory;
        }

        *p = old;
    }
    return i;
}



void HariMain(void) 
{
    BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
    char *mouse, s[40], keybuf[32], mousebuf[128];
    int mx, my, d;
    struct MOUSE_DEC mdec;
    mx = 100;
    my = 100;

    unsigned int i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);

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
    sprintf(s, "Memory Size: %dM", i);
    str_renderer8(binfo, COL8_FFFFFF, 0, 32, s);

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

                    sprintf(s, "(%d   , %d)", mx, my);
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
