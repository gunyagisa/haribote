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

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
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

  char c, tc, tbc;
  if (act != 0) {
    tc = COL8_FFFFFF;
    tbc = COL8_000084;
  } else {
    tc = COL8_C6C6C6;
    tbc = COL8_848484;
  }

  boxfill8(buf, xsize, COL8_C6C6C6, 0,                 0, xsize - 1, 0);
  boxfill8(buf, xsize, COL8_FFFFFF, 1,                 1, xsize - 2, 1);
  boxfill8(buf, xsize, COL8_C6C6C6, 0,                 0, 0, ysize - 1);
  boxfill8(buf, xsize, COL8_FFFFFF, 1,                 1, 1, ysize - 2);
  boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
  boxfill8(buf, xsize, tbc,         3, 3, xsize - 4, 20);
  boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize -1);
  str_renderer8(buf, xsize, tc, 24, 4, title);

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

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
  int x1 = x0 + sx, y1 = y0 + sy;
  boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
  boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
  boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
  boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
  boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
  boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
  boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
}

void str_renderer_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
  boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
  str_renderer8(sht->buf, sht->bxsize, c, x, y, s);
  sheet_refresh(sht, x, y, x + l * 8, y + 16);
}



void HariMain(void) 
{

  struct BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
  char s[40];
  struct FIFO32 fifo;
  int fifobuf[128];
  int mx, my, d;
  struct MOUSE_DEC mdec;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_b[3];
  unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_win_b;

  struct TASK *task_a, *task_b[3];
  struct TIMER *timer;

  static char keytable[0x54] = {
    0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.'
  };

  unsigned int memtotal;


  init_gdtidt();
  init_pic();
  io_sti();
  fifo32_init(&fifo, 128, fifobuf, 0);

  init_pit();         // PIT configure

  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

  io_out8(PIC0_IMR, 0xf8);
  io_out8(PIC1_IMR, 0xef);

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  init_palette();	//configure color setting

  shtctl = shtctl_init(memman, (unsigned char *)binfo->vram, binfo->scrnx, binfo->scrny);
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 0);

  // sht_back
  sht_back = sheet_alloc(shtctl);
  buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  init_screen(buf_back, binfo->scrnx, binfo->scrny);

  // sht_win
  sht_win = sheet_alloc(shtctl);
  buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
  sheet_setbuf(sht_win, buf_win, 144, 52, -1);
  make_window8(buf_win, 144, 52, "task_a", 1);
  make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
  int cursor_x, cursor_c;
  cursor_x = 8;
  cursor_c = COL8_FFFFFF;
  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  settimer(timer, 50);

  // sht_mouse
  sht_mouse = sheet_alloc(shtctl);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_mouse_cursor8((char *)buf_mouse, 99);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(sht_back, 0, 0);
  sheet_slide(sht_win_b[0], 168, 56);
  sheet_slide(sht_win_b[1], 8, 116);
  sheet_slide(sht_win_b[2], 168, 116);
  sheet_slide(sht_win, 8, 56);
  sheet_slide(sht_mouse, mx, my);
  sheet_updown(sht_back, 0);
  sheet_updown(sht_win_b[0], 1);
  sheet_updown(sht_win_b[1], 2);
  sheet_updown(sht_win_b[2], 3);
  sheet_updown(sht_win, 4);
  sheet_updown(sht_mouse, 5);
  sprintf(s, "(%d, %d)", mx, my);
  str_renderer_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
  sprintf(s, "Memory Size: %dMB  free: %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
  str_renderer_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

  for (;;) {
    io_cli();
    if (fifo32_status(&fifo) == 0) {
      task_sleep(task_a);
      io_sti();
    } else {
      d = fifo32_get(&fifo);
      io_sti();
      if (256 <= d && d <= 511) { //keyboard
        sprintf(s, "%x", d - 256);
        str_renderer_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
        if (d < 0x54 + 256) {
          if (keytable[d - 256] != 0 && cursor_x < 128) {
            s[0] = keytable[d - 256];
            s[1] = 0;
            str_renderer_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
            cursor_x += 8;
          }
        }
        if (d == 256 + 0x0e && cursor_x > 8) {
          str_renderer_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
          cursor_x -= 8;
        }

        boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
        sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);

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
      } else if (d <= 1){
        if (d != 0) {
          timer_init(timer, &fifo, 0);
          cursor_c = COL8_000000;
        } else {
          timer_init(timer, &fifo, 1);
          cursor_c = COL8_FFFFFF;
        }
        settimer(timer, 50);
        boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
        sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
      }
    }
  }
}
