#include "bootpack.h"
#include "dsctbl.h"


void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
  char s[2];
  s[0] = chr;
  s[1] = 0;
  if (s[0] == 0x09) { // tab
    for (;;) {
      str_renderer_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {
        cons_newline(cons);
      }
      if (((cons->cur_x - 8) & 0x1f) == 0) {
        break;
      }
    }
  } else if (s[0] == 0x0a) { // line feed
    cons_newline(cons);
  } else if (s[0] == 0x0d) { // CR
    // nothing
  } else { // normal chara
    str_renderer_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
    if (move != 0) {
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {
        cons_newline(cons);
      }
    }
  }
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
  if (strcmp(cmdline, "mem") == 0) {
    cmd_mem(cons, memtotal);
  } else if (strcmp(cmdline, "clear") == 0) {
    cmd_clear(cons);
  }else if (strcmp(cmdline, "ls") == 0) {
    cmd_ls(cons);
  } else if (strncmp(cmdline, "cat ", 4) == 0) {
    cmd_cat(cons, fat, cmdline);
  } else if (cmdline[0] != 0) {
    if (cmd_app(cons, fat, cmdline) == 0) {
      str_renderer_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, "Bad command", 11);
      cons_newline(cons);
      cons_newline(cons);
    }
  }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  char s[30];
  sprintf(s, "total %dMB", memtotal / (1024 * 1024));
  str_renderer_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
  cons_newline(cons);
  sprintf(s, "free %dKB", memman_total(memman) / 1024);
  str_renderer_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
  cons_newline(cons);
  cons_newline(cons);
}

void cmd_clear(struct CONSOLE *cons)
{
  struct SHEET *sheet = cons->sht;
  for (int y = 28; y < 28+128; y++) {
    for (int x = 8; x < 8 + 240; x++) {
      sheet->buf[x + y * sheet->bxsize] = COL8_000000;
    }
  }
  sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
  cons->cur_y = 28;
}

void cmd_ls(struct CONSOLE *cons)
{
  struct FILEINFO *finfo = (struct FILEINFO *) (DISKIMG_ADDR + 0x002600);
  char s[30];
  for (int i = 0; i < 224; i++) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].name[0] != 0xe5) {
      if ((finfo[i].type & 0x18) == 0) {
        sprintf(s, "filename.ext        %d", finfo[i].size);
        for (int j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[9] = finfo[i].ext[0];
        s[10] = finfo[i].ext[1];
        s[11] = finfo[i].ext[2];
        str_renderer_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
        cons_newline(cons);
      }
    }
  }
  cons_newline(cons);
}

void cmd_cat(struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct FILEINFO *finfo = file_search(cmdline + 4, (struct FILEINFO *) (DISKIMG_ADDR + 0x002600), 224);
  char *p;
  
  if (finfo != 0) {
    p = (char *) memman_alloc_4k(memman, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (DISKIMG_ADDR + 0x003e00));
    for (int i = 0; i < finfo->size; i++) {
      cons_putchar(cons, p[i], 1);
    }
    memman_free_4k(memman, (int) p, finfo->size);
  } else {
    str_renderer_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
    cons_newline(cons);
  }
  cons_newline(cons);
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) GDT_ADDR;
  char *p, name[18];

  int i;
  for (i = 0; i < 13; i++) {
    if (cmdline[i] <= ' ') break;
    name[i] = cmdline[i];
  }
  name[i] = 0;
  struct FILEINFO *finfo = file_search(name, (struct FILEINFO *) (DISKIMG_ADDR + 0x002600), 224);

  if (finfo == 0 && name[i - 1] != '.') {
    name[i    ] = '.';
    name[i + 1] = 'H';
    name[i + 2] = 'R';
    name[i + 3] = 'B';
    name[i + 4] = 0;
    finfo = file_search(name, (struct FILEINFO *) (DISKIMG_ADDR + 0x002600), 224);
  }

  if (finfo != 0) {
    p = (char *) memman_alloc_4k(memman, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (DISKIMG_ADDR + 0x003e00));
    set_sgmntdsc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
    farcall(0, 1003 * 8);
    cons_newline(cons);
    memman_free_4k(memman, (int) p, finfo->size);
    return 1;
  }
  return 0;
}

void console_task(struct SHEET *sht, unsigned int memtotal)
{
  struct TIMER *timer;
  struct TASK *task = task_now();
  int i, fifobuf[128];
  char cmdline[30];
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  int *fat = (int *) memman_alloc_4k(memman, 4 * 2800);

  struct CONSOLE cons;
  cons.sht = sht;
  cons.cur_x = 8;
  cons.cur_y = 28;
  cons.cur_c  = -1;

  *((int *) 0xfec) = (int) &cons;

  
  fifo32_init(&task->fifo, 128, fifobuf, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  settimer(timer, 50);
  file_readfat(fat, (unsigned char *) (DISKIMG_ADDR + 0x000200));

  cons_putchar(&cons, '>', 1);

  for (;;) {
    io_cli();
    if (fifo32_status(&task->fifo) == 0) {
      task_sleep(task);
      io_sti();
    } else {
      i = fifo32_get(&task->fifo);
      io_sti();
      if ( i <= 1) {
        if (i != 0) {
          timer_init(timer, &task->fifo, 0);
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_FFFFFF;
          }
        } else {
          timer_init(timer, &task->fifo, 1);
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_000000;
          }
        }
        settimer(timer, 50);
      }
      if (i == 2) { // cursor on
        cons.cur_c = COL8_FFFFFF;
      }
      if (i == 3) { // cursor off
        boxfill8(sht->buf, sht->bxsize, COL8_000000, cons.cur_x, 28, cons.cur_x + 7, 43);
        cons.cur_c = -1;
      }
      if (256 <= i && i <= 511) {
        if (i == 8 + 256) { // backspace
          if (cons.cur_x > 16) {
            cons_putchar(&cons, ' ', 0);
            cons.cur_x -= 8;
          }
        } else if (i == 10 + 256) {
          cons_putchar(&cons, ' ', 0);
          cmdline[cons.cur_x / 8 - 2] = 0;
          cons_newline(&cons);
          cons_runcmd(cmdline, &cons, fat, memtotal);
          cons_putchar(&cons, '>', 1);
        } else {
          if (cons.cur_x < 240) {
            cmdline[cons.cur_x / 8 - 2] = i - 256;
            cons_putchar(&cons, i-256, 1);
          }
        }
      }
      if (cons.cur_c >= 0) {
        boxfill8(sht->buf, sht->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
      }
      sheet_refresh(sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
    }
  }
}

void cons_newline(struct CONSOLE *cons)
{
  if (cons->cur_y < 28 + 112) {
    cons->cur_y += 16;
  } else {
    for (int y = 28;y < 28 + 112;y++) {
      for (int x = 8;x < 8 + 240;x++) {
        cons->sht->buf[x + y * cons->sht->bxsize] = cons->sht->buf[x + (y + 16) * cons->sht->bxsize];
      }
    }
    for (int y = 28 + 112;y < 28 + 128;y++) {
      for (int x = 8; x < 8 + 240;x++) {
        cons->sht->buf[x + y * cons->sht->bxsize] = COL8_000000;
      }
    }
    sheet_refresh(cons->sht, 8, 28, 8 + 240, 28 + 128);
  }
  cons->cur_x = 8;
}
