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

void HariMain(void) 
{
  struct BOOTINFO *binfo = (BOOTINFO *) BOOTINFO_ADDR;
  char s[40];
  struct FIFO32 fifo, keycmd;
  int fifobuf[128], keycmd_buf[32], *cons_fifo[2];
  int mx, my, d;
  struct MOUSE_DEC mdec;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht_cons[2], *sht, *key_win;
  unsigned char *buf_back, buf_mouse[256], *buf_cons[2];

  struct TASK *task_a, *task_cons[2], *task;

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
  for (int i = 0; i < 2; i++) {
    sht_cons[i] = sheet_alloc(shtctl);
    buf_cons[i] = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1);
    make_window8(buf_cons[i], 256, 165, "console", 0);
    make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
    task_cons[i] = task_alloc();
    task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
    task_cons[i]->tss.eip = (int) &console_task;
    task_cons[i]->tss.es = 1 * 8;
    task_cons[i]->tss.cs = 2 * 8;
    task_cons[i]->tss.ss = 1 * 8;
    task_cons[i]->tss.ds = 1 * 8;
    task_cons[i]->tss.fs = 1 * 8;
    task_cons[i]->tss.gs = 1 * 8;
    *((int *) (task_cons[i]->tss.esp + 4)) = (int) sht_cons[i];
    *((int *) (task_cons[i]->tss.esp + 8)) = memtotal;
    task_run(task_cons[i], 2, 2);
    sht_cons[i]->task = task_cons[i];
    sht_cons[i]->flags |= 0x20;
    cons_fifo[i] = (int *) memman_alloc_4k(memman, 128 * 4);
    fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
  }


  // sht_mouse
  sht_mouse = sheet_alloc(shtctl);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_mouse_cursor8((char *)buf_mouse, 99);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(sht_back, 0, 0);
  sheet_slide(sht_cons[1], 56, 6);
  sheet_slide(sht_cons[0], 8, 2);
  sheet_slide(sht_mouse, mx, my);
  sheet_updown(sht_back, 0);
  sheet_updown(sht_cons[1], 1);
  sheet_updown(sht_cons[0], 2);
  sheet_updown(sht_mouse, 3);
  key_win = sht_cons[0];
  keywin_on(key_win);

  int mmx = -1, mmy = -1;

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
      task_sleep(task_a);
      io_sti();
    } else {
      d = fifo32_get(&fifo);
      io_sti();
      if (key_win->flags == 0) {
        key_win = shtctl->sheets[shtctl->top - 1];
        keywin_on(key_win);
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
        if (s[0] != 0) {
          fifo32_put(&key_win->task->fifo, s[0] + 256);
        }
        if (d == 256 + 0x0f) {
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
        if (d == 256 + 0x3b && key_shift != 0 && key_shift != 0) {
          task = key_win->task;
          if (task != 0 && task->tss.ss0 != 0) {
            cons_putstr0(task->cons, "\nBreak(KEY) :\n");
            io_cli();
            task->tss.eax = (int) &(task->tss.esp0);
            task->tss.eip = (int) end_app_asm;
            io_sti();
          }
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
          sheet_slide(sht_mouse, mx, my);
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
              sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
              mmx = mx;
              mmy = my;
            }
          } else {
            mmx = -1;
          }
        }
      }
    }
  }
}
