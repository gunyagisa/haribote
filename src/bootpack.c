#include "bootpack.h"
#include "graphic.h" 
#include "dsctbl.h"
#include "interrupt.h"
#include "mouse.h"
#include "keyboard.h"
#include "mysprintf.h" 
#include "memory.h"
#include "sheet.h"
#include "dsctbl.h"


void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
  static char closebtn[14][16] = {
    "00000000000000$@",
    "0QQQQQQQQQQQQQ$@",
    "0QQQQQQQQQQQQQ$@", 
    "0QQQ@@QQQQ@@QQ$@", 
    "0QQQQ@@QQ@@QQQ$@", 
    "0QQQQQ@@@@QQQQ$@", 
    "0QQQQQQ@@QQQQQ$@", 
    "0QQQQQ@@@@QQQQ$@",
    "0QQQQ@@QQ@@QQQ$@",
    "0QQQ@@QQQQ@@QQ$@",
    "0QQQQQQQQQQQQQ$@",
    "0QQQQQQQQQQQQQ$@",
    "0$$$$$$$$$$$$$$@",
    "@@@@@@@@@@@@@@@@",
  };
  boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
  boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
  boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
  boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
  boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
  boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
  boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize -1);
  str_renderer8(buf, xsize, COL8_FFFFFF, 24, 4, title);

  unsigned char c;
  for ( int y = 0;y < 14;++y) {
    for (int x = 0;x < 16;++x) {
      c = closebtn[y][x];
      if (c == '@') {
        c = COL8_000000;
      } else if ( c == '$') {
        c = COL8_848484;
      } else if (c == 'Q') {
        c = COL8_C6C6C6;
      } else {
        c = COL8_FFFFFF;
      }
      buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
    }
  }
}


void str_renderer_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
  boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
  str_renderer8(sht->buf, sht->bxsize, c, x, y, s);
  sheet_refresh(sht, x, y, x + l * 8, y + 16);
}

struct TSS32 {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

void task_b_main(void)
{
  for (;;) { io_hlt(); }
}

void HariMain(void) 
{

  BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
  char s[40];
  FIFO32 fifo;
  int fifobuf[128];
  int mx, my, d;
  struct MOUSE_DEC mdec;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht_win;
  unsigned char *buf_back, buf_mouse[256], *buf_win;
  struct TIMER *timer1, *timer2, *timer3;
  static char keytable[0x54] = {
      0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.'
  };

  int task_b_esp;
  struct TSS32 tss_a, tss_b;

  unsigned int memtotal;
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  io_cli();
  init_gdtidt();
  init_pic();
  io_sti();
  fifo32_init(&fifo, 128, fifobuf);

  SEGMENT_DISCRIPTOR *gdt = (SEGMENT_DISCRIPTOR *) GDT_ADDR;

  init_pit();         // PIT configure

  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

  io_out8(PIC0_IMR, 0xf8);
  io_out8(PIC1_IMR, 0xef);

  timer1 = timer_alloc();
  timer_init(timer1, &fifo, 10);
  settimer(timer1, 1000);
  timer2 = timer_alloc();
  timer_init(timer2, &fifo, 3);
  settimer(timer2, 300);
  timer3 = timer_alloc();
  timer_init(timer3, &fifo, 1);
  settimer(timer3, 50);

  init_palette();	//configure color setting

  shtctl = shtctl_init(memman, (unsigned char *)binfo->vram, binfo->scrnx, binfo->scrny);
  sht_back = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  sht_win = sheet_alloc(shtctl);

  buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);

  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  sheet_setbuf(sht_win, buf_win, 160, 52, -1);
  init_screen(buf_back, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8((char *)buf_mouse, 99);
  make_window8(buf_win, 160, 52, "window");

  sheet_slide(sht_back, 0, 0);
  sheet_slide(sht_mouse, mx, my);
  sheet_slide(sht_win, 80, 72);

  sheet_updown(sht_back, 0);
  sheet_updown(sht_mouse, 2);
  sheet_updown(sht_win, 1);

  sprintf(s, "(%d, %d)", mx, my);
  str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, 0, s);
  sprintf(s, "Memory Size: %dMB  free: %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
  str_renderer8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, 32, s);
  sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

  tss_a.ldtr = 0;
  tss_a.iomap = 0x40000000;
  tss_b.ldtr = 0;
  tss_a.iomap = 0x40000000;
  set_sgmntdsc(gdt + 3, 103, (int) &tss_a, AR_TSS32);
  set_sgmntdsc(gdt + 4, 103, (int) &tss_b, AR_TSS32);
  load_tr(3 * 8);
  task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
  tss_b.eip = (int) &task_b_main;
  tss_b.eflags = 0x00000202; 	/* IF = 1 */
  tss_b.eax = 0;
  tss_b.ecx = 0;
  tss_b.edx = 0;
  tss_b.ebx = 0;
  tss_b.esp = task_b_esp;
  tss_b.ebp = 0;
  tss_b.esi = 0;
  tss_b.edi = 0;
  tss_b.es = 1 * 8;
  tss_b.cs = 2 * 8;
  tss_b.ss = 1 * 8;
  tss_b.ds = 1 * 8;
  tss_b.fs = 1 * 8;
  tss_b.gs = 1 * 8;

  for (;;) {
    io_cli();
    if (fifo32_status(&fifo) == 0) {
      io_stihlt();
    } else {
      d = fifo32_get(&fifo);
      if (256 <= d && d <= 511) { //keyboard
        sprintf(s, "%x", d - 256);
        str_renderer_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
        if (d < 256 + 0x54) { 
          if (keytable[d - 256] != 0) {
            s[0] = keytable[d - 256];
            s[1] = 0;
            str_renderer_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 1);
          }
        }
      } else if (512 <= d && d <= 767) { //mouse
        if (mouse_decode(&mdec, d - 512) != 0) {
          sprintf(s, "[lcr %d %d]", mdec.x, mdec.y);
          if ((mdec.btn & 0x01) != 0)
            s[1] = 'L';
          if ((mdec.btn & 0x02) != 0)
            s[3] = 'R';
          if ((mdec.btn & 0x04) != 0)
            s[2] = 'C';
          str_renderer_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

          mx += mdec.x;
          my += mdec.y;

          if (mx < 0)
            mx = 0;
          if (my < 0)
            my = 0;
          if (mx > binfo->scrnx - 1)
            mx = binfo->scrnx - 1;
          if (my > binfo->scrny - 1)
            my = binfo->scrny - 1;

          sprintf(s, "(%d, %d)", mx, my);
          str_renderer_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
          sheet_slide(sht_mouse, mx, my);
          if ((mdec.btn & 0x01) != 0) {
            // move sht_win
            sheet_slide(sht_win, mx - 80, my - 8);
          }
        }
      } else if (d == 10) {
        str_renderer_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
        taskswitch();
      } else if (d == 3) {
        str_renderer_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
      } else {
        if (d != 0) {
          timer_init(timer3, &fifo, 0);
          boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
        } else {
          timer_init(timer3, &fifo, 1);
          boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
        }
        settimer(timer3, 50);
        sheet_refresh(sht_back, 8, 96, 16, 112);
      }
    }
  }
}
