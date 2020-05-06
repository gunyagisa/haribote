#include "bootpack.h"
#include "graphic.h" 
#include "dsctbl.h"
#include "interrupt.h"
#include "mouse.h"
#include "keyboard.h"
#include "dsctbl.h"


#define KEYCMD_LED      0xed

void keywin_off(struct SHEET *key_win)
{
  change_wtitle8(key_win, 0);
  if((key_win->flags & 0x20) != 0) {
    fifo32_put(&key_win->task->fifo, 3);
  }
}

void keywin_on(struct SHEET *key_win)
{
  change_wtitle8(key_win, 1);
  if ((key_win->flags & 0x20) != 0) {
    fifo32_put(&key_win->task->fifo, 2);
  }
}

struct SHEET * open_console(struct SHTCTL *shtctl, int memtotal)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHEET *sht = sheet_alloc(shtctl);
  unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
  struct TASK *task = task_alloc();
  int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
  sheet_setbuf(sht, buf, 256, 165, -1);
  make_window8(buf, 256, 165, "console", 0);
  make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
  task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
  task->tss.esp = task->cons_stack + 64 * 1024 - 12;
  task->tss.eip = (int) &console_task;
  task->tss.es = 1 * 8;
  task->tss.cs = 2 * 8;
  task->tss.ss = 1 * 8;
  task->tss.ds = 1 * 8;
  task->tss.fs = 1 * 8;
  task->tss.gs = 1 * 8;
  *((int *) (task->tss.esp + 4)) = (int) sht;
  *((int *) (task->tss.esp + 8)) =(int) memtotal;
  task_run(task, 2, 2);
  sht->task = task;
  sht->flags |= 0x20;
  fifo32_init(&task->fifo, 128, cons_fifo, task);
  return sht;
}

void close_constask(struct TASK *task)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  task_sleep(task);
  memman_free_4k(memman, task->cons_stack, 64 * 1024);
  memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
  task->flags = 0;
}

void close_console(struct SHEET *sht)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct TASK *task = sht->task;
  memman_free_4k(memman, (int) sht->buf, 256 * 165);
  sheet_free(sht);
  close_constask(task);
}

void HariMain(void) 
{
  struct BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
  char s[40];
  struct FIFO32 fifo, keycmd;
  int fifobuf[128], keycmd_buf[32];
  int mx, my, d;
  struct MOUSE_DEC mdec;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht, *key_win;
  unsigned char *buf_back, buf_mouse[256];

  struct TASK *task_a, *task;

  static char keytable0[0x80] = {
    0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',   0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',   0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0x5c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x5c, 0, 0
  };

  static char keytable1[0x80] = {
    0,   0, '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~',   0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{',   0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*',   0,   0, '}', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '?',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, '_', 0, 0, 0, 0, 0, 0, 0, 0, 0, '|', 0, 0
  };
  int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

  unsigned int memtotal;

  init_gdtidt();
  init_pic();
  io_sti();
  fifo32_init(&fifo, 128, fifobuf, 0);
  fifo32_init(&keycmd, 32, keycmd_buf, 0);

  *((int *) 0xfec) = (int) &fifo;

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
  *((int *) 0xfe4) = (int) shtctl;
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 0);

  // sht_back
  sht_back = sheet_alloc(shtctl);
  buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  init_screen(buf_back, binfo->scrnx, binfo->scrny);

  // sht_cons
  key_win = open_console(shtctl, memtotal);
  
  // sht_mouse
  sht_mouse = sheet_alloc(shtctl);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_mouse_cursor8((char *)buf_mouse, 99);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(sht_back, 0, 0);
  sheet_slide(key_win, 32, 4);
  sheet_slide(sht_mouse, mx, my);
  sheet_updown(sht_back, 0);
  sheet_updown(key_win, 1);
  sheet_updown(sht_mouse, 2);
  keywin_on(key_win);

  int mmx = -1, mmy = -1, mmx2 = 0;
  int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;

  fifo32_put(&keycmd, KEYCMD_LED);
  fifo32_put(&keycmd, key_leds);

  for (;;) {
    if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
      keycmd_wait = fifo32_get(&keycmd);
      wait_KBC_sendready();
      io_out8(PORT_KEYDATA, keycmd_wait);
    }
    io_cli();
    if (fifo32_status(&fifo) == 0) {
      if (new_mx >= 0) {
        io_sti();
        sheet_slide(sht_mouse, new_mx, new_my);
        new_mx = -1;
      } else if (new_wx != 0x7fffffff) {
        io_sti();
        sheet_slide(sht, new_wx, new_wy);
        new_wx = 0x7fffffff;
      } else {
        task_sleep(task_a);
        io_sti();
      }
    } else {
      d = fifo32_get(&fifo);
      io_sti();
      if (key_win != 0 && key_win->flags == 0) {
        if (shtctl->top == 1) {
          key_win = 0;
        } else {
          key_win = shtctl->sheets[shtctl->top - 1];
          keywin_on(key_win);
        }
      }
      if (256 <= d && d <= 511) { //keyboard
        if (d < 0x80 + 256) { //normal chara
          if (key_shift == 0) {
            s[0] = keytable0[d - 256];
          } else {
            s[0] = keytable1[d - 256];
          }
        } else {
          s[0] = 0;
        }
        if ('A' <= s[0] && s[0] <= 'Z') {
          if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) {
            s[0] += 0x20;
          }
        }
        if (s[0] != 0 && key_win != 0) {
          fifo32_put(&key_win->task->fifo, s[0] + 256);
        }
        if (d == 256 + 0x0f && key_win != 0) {
          //tab
          keywin_off(key_win);
          int j = key_win->height - 1;
          if (j == 0) {
            j = shtctl->top - 1;
          }
          key_win = shtctl->sheets[j];
          keywin_on(key_win);
        }
        if (d == 256 + 0x3a) { // capslock
          key_leds ^= 4;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        if (d == 256 + 0x3b && key_shift != 0 && key_win != 0) {
          task = key_win->task;
          if (task != 0 && task->tss.ss0 != 0) {
            cons_putstr0(task->cons, "\nBreak(KEY) :\n");
            io_cli();
            task->tss.eax = (int) &(task->tss.esp0);
            task->tss.eip = (int) end_app_asm;
            io_sti();
          }
        }
        if (d == 256 + 0x3c && key_shift != 0) {
          if (key_win != 0)
            keywin_off(key_win);
          key_win = open_console(shtctl, memtotal);
          sheet_slide(key_win, 64, 40);
          sheet_updown(key_win, shtctl->top);
          keywin_on(key_win);
        }
        if (d == 256 + 0x45) { //numlock
          key_leds ^= 2;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        if (d == 256 + 0x46) { // scrolllock
          key_leds ^= 1;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        if (d == 256 + 0xfa) { // key data was accepted
          keycmd_wait = -1;
        }
        if (d == 256 + 0xfe) { // key data was not accepted
          wait_KBC_sendready();
          io_out8(PORT_KEYDATA, keycmd_wait);
        }
        if (d == 256 + 0x2a) {
          key_shift |= 1;
        }
        if (d == 256 + 0x36) {
          key_shift |= 2;
        }
        if (d == 256 + 0xaa) {
          key_shift &= ~1;
        }
        if (d == 256 + 0xb6) {
          key_shift &= ~2;
        }
        if (d == 256 + 0x57 && shtctl->top > 2) { // F11
          sheet_updown(shtctl->sheets[1], shtctl->top - 1);
        }
      } else if (512 <= d && d <= 767) { //mouse
        if (mouse_decode(&mdec, d - 512) != 0) {

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
          new_mx = mx;
          new_my = my;
          if ((mdec.btn & 0x01) != 0) {
            int x, y;
            if(mmx < 0) {
              // normal mode
              for (int j = shtctl->top - 1; j > 0; j--) {
                sht = shtctl->sheets[j];
                x = mx - sht->vx0;
                y = my - sht->vy0;
                if (0 <= x && x <= sht->bxsize && 0 <= y && y <= sht->bysize) {
                  if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                    sheet_updown(sht, shtctl->top - 1);
                    if (sht != key_win) {
                      keywin_off(key_win);
                      key_win = sht;
                      keywin_on(key_win);
                    }
                    if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
                      // prepare for window move
                      mmx = mx;
                      mmy = my;
                      mmx2 = sht->vx0;
                      new_wy = sht->vy0;
                    }
                    if (sht->bxsize - 21 <= x && x <= sht->bxsize && sht->bysize - 5 && y < 19) {
                      if((sht->flags & 0x10) != 0) {
                        task = sht->task;
                        cons_putstr0(task->cons, "\nBreak(mouse) :\n");
                        io_cli();
                        task->tss.eax = (int) &(task->tss.esp0);
                        task->tss.eip = (int) end_app_asm;
                        io_cli();
                      }
                    }
                    break;
                  }
                }
              }
            } else {
              // window move
              x = mx - mmx;
              y = my - mmy;
              new_wx = (mmx2 + x + 2) & ~3;
              new_wy = new_wy + y;
              mmy = my;
            }
          } else {
            mmx = -1;
            if (new_wx != 0x7fffffff) {
              sheet_slide(sht, new_wx, new_wy);
              new_wx = 0x7fffffff;
            }
          }
        }
      } else if (768 <= d && d <= 1023) {
        close_console(shtctl->sheets0 + (d - 768));
      }
    }
  }
}
