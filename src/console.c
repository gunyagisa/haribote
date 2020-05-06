#include "bootpack.h"
#include "dsctbl.h"
#include "sheet.h"


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
    str_renderer_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1); if (move != 0) { cons->cur_x += 8;
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
  } else if (strcmp(cmdline, "exit") == 0) {
    cmd_exit(cons, fat);
  } else if (cmdline[0] != 0) {
    if (cmd_app(cons, fat, cmdline) == 0) {
      cons_putstr0(cons, "Bad command.\n\n");
    }
  }
}

void cmd_exit(struct CONSOLE *cons, int *fat)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct TASK *task = task_now();
  struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0xfe4);
  struct FIFO32 *fifo = (struct FIFO32 *) *((int *) 0xfec);

  timer_cancel(cons->timer);
  memman_free_4k(memman, (int) fat, 4 * 2880);
  io_cli();
  fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);
  io_sti();
  for (;;) task_sleep(task);
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  char s[30];
  sprintf(s, "total %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
  cons_putstr0(cons, s);
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
        sprintf(s, "filename.ext        %d\n", finfo[i].size);
        for (int j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[9] = finfo[i].ext[0];
        s[10] = finfo[i].ext[1];
        s[11] = finfo[i].ext[2];
        cons_putstr0(cons, s);
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
    cons_putstr0(cons, "File not found.\n");
  }
  cons_newline(cons);
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) GDT_ADDR;
  char *p, name[18], *q;
  struct TASK *task = task_now();
  int segsiz, datsiz, esp, dathrb;

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
    if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
      segsiz = *((int *) (p + 0x0000));
      esp    = *((int *) (p + 0x000c));
      datsiz = *((int *) (p + 0x0010));
      dathrb = *((int *) (p + 0x0014));
      q = (char *) memman_alloc_4k(memman, segsiz);
      task->ds_base = (int) q;
      set_sgmntdsc(gdt + task->sel / 8 + 1000, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60); // code segment
      set_sgmntdsc(gdt + task->sel / 8 + 2000, segsiz - 1, (int) q, AR_DATA32_RW + 0x60); // data segment
      for (int i = 0; i < datsiz; i++) {
        q[esp + i] = p[dathrb + i];
      }
      start_app(0x1b, task->sel + 1000 * 8, esp, task->sel + 2000 * 8, &(task->tss.esp0));
      struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0xfe4);
      struct SHEET *sht;
      for (int i = 0; i < SHEET_MAX; i++) {
        sht = &(shtctl->sheets0[i]);
        if((sht->flags & 0x11) == 0x11 && sht->task == task) {
          sheet_free(sht);
        }
      }
      timer_cancelall(&task->fifo);
      memman_free_4k(memman, (int) q, segsiz);
    } else {
      cons_putstr0(cons, ".hrb file format error.\n");
    }
    memman_free_4k(memman, (int) p, finfo->size);
    cons_newline(cons);
    return 1;
  }
  return 0;
}

void console_task(struct SHEET *sht, unsigned int memtotal)
{
  struct TASK *task = task_now();
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  int i, *fat = (int *) memman_alloc_4k(memman, 4 * 2800);
  char cmdline[30];

  struct CONSOLE cons;
  cons.sht = sht;
  cons.cur_x = 8;
  cons.cur_y = 28;
  cons.cur_c  = -1;
  task->cons = &cons;

  cons.timer = timer_alloc();
  timer_init(cons.timer, &task->fifo, 1);
  settimer(cons.timer, 50);
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
          timer_init(cons.timer, &task->fifo, 0);
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_FFFFFF;
          }
        } else {
          timer_init(cons.timer, &task->fifo, 1);
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_000000;
          }
        }
        settimer(cons.timer, 50);
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

void cons_putstr0(struct CONSOLE *cons, char *s)
{
  while (*s != 0) {
    cons_putchar(cons, *s, 1);
    s++;
  }
}

void cons_putstr1(struct CONSOLE *cons, char *s, int n)
{
  for (int i = 0; i < n; i++) {
    cons_putchar(cons, s[i], 1);
  }
}

void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
  int dx, dy, len, x, y;

  dx = x1 - x0;
  dy = y1 - y0;
  x = x0 << 10;
  y = y0 << 10;

  if (dx < 0) dx = -dx;
  if (dy < 0) dy = -dy;
  
  if (dx >= dy) {
    len = dx + 1;
    if (x0 > x1) 
      dx = -1024;
    else 
      dx = 1024;

    if (y0 <= y1)
      dy = ((y1 - y0 + 1) << 10) / len;
    else 
      dy = ((y1 - y0 - 1) << 10) / len;
  } else {
    len = dy + 1;
    if (y0 > y1)
      dy = -1024;
    else
      dy = 1024;

    if(x0 >= x1)
      dx = ((x1 - x0 + 1) << 10) / len;
    else 
      dx = ((x1 - x0 - 1) << 10) / len;
  }

  for (int i = 0; i < len; i++) {
    sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
    x += dx;
    y += dy;
  }
}

int * hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
  struct TASK *task = task_now();
  struct CONSOLE *cons = task->cons;
  int ds_base = task->ds_base;
  struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0xfe4);
  int *reg = &eax + 1; // pushad value
  struct SHEET *sht;
  
  if (edx == 1) {
    cons_putchar(cons, eax & 0xff, 1);
  } else if (edx == 2) {
    cons_putstr0(cons, (char *) ebx + ds_base);
  } else if (edx == 3) {
    cons_putstr1(cons, (char *) ebx + ds_base, ecx);
  } else if (edx == 4) {
    return &(task->tss.esp0);
  } else if (edx == 5) { // api_openwin
    sht = sheet_alloc(shtctl);
    sht->task = task;
    sht->flags |= 0x10;
    sheet_setbuf(sht, (unsigned char *) ebx + ds_base, esi, edi, eax);
    make_window8((unsigned char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
    sheet_slide(sht, (shtctl->xsize - esi) / 2 & ~3, (shtctl->ysize - edi) / 2);
    sheet_updown(sht, shtctl->top);
    reg[7] = (int) sht;
  } else if (edx == 6) { // api_putstrwin
    sht = (struct SHEET *) (ebx & 0xfffffffe);
    str_renderer8(sht->buf, sht->bxsize, eax, esi, edi, (char *) ebp + ds_base);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
    }
  } else if (edx == 7) { // api_boxfilliwin
    sht = (struct SHEET *) (ebx & 0xfffffffe);
    boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    }
  } else if (edx == 8) { // api_initmalloc
    memman_init((struct MEMMAN *) (ebx + ds_base));
    ecx &= 0xfffffffe;
    reg[7] = memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
  } else if (edx == 9) { // api_malloc
    ecx = (ecx + 0x0f) & 0xfffffff0;
    reg[7] = memman_alloc_4k((struct MEMMAN *) (ebx + ds_base), ecx);
  } else if (edx == 10) { // api_free
    ecx = (ecx + 0x0f) & 0xfffffff0;
    memman_free_4k((struct MEMMAN *) (ebx + ds_base), eax, ecx);
  } else if (edx == 11) { // api_point
    sht = (struct SHEET *) (ebx & 0xfffffffe);
    sht->buf[esi + edi * sht->bxsize] = eax;
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
    }
  } else if (edx == 12) { // api_refreshwin
    sht = (struct SHEET *) ebx;
    sheet_refresh(sht, eax, ecx, esi, edi);
  } else if (edx == 13) {
    sht = (struct SHEET *) (ebx & 0xfffffffe);
    hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    }
  } else if (edx == 14) { //  api_closewin
    sheet_free((struct SHEET *) ebx);
  } else if (edx == 15) {
    for (;;) {
      io_cli();
      if (fifo32_status(&task->fifo) == 0) {
        if (eax != 0) {
          task_sleep(task);
        } else {
          io_sti();
          reg[7] = -1;
          return 0;
        }
      }
      int i = fifo32_get(&task->fifo);
      io_sti();
      if (i <= 1) {
        timer_init(cons->timer, &task->fifo, 1);
        settimer(cons->timer, 50);
      }
      if (i == 2) {
        cons->cur_c = COL8_FFFFFF;
      }
      if (i == 3) {
        cons->cur_c = -1;
      }
      if (256 <= i) {
        reg[7] = i - 256;
        return 0;
      }
    }
  } else if (edx == 16) {
    reg[7] = (int) timer_alloc();
    ((struct TIMER *) reg[7])->flags2 = 1; // auto cancel
  } else if (edx == 17) {
    timer_init((struct TIMER *) ebx, &task->fifo, eax + 256);
  } else if (edx == 18) {
    settimer((struct TIMER *) ebx, eax);
  } else if (edx == 19) {
    timer_free((struct TIMER *) ebx);
  } else if (edx  == 20) { // api_beep
    if (eax == 0) {
      int i = io_in8(0x61);
      io_out8(0x61, i & 0xd);
    } else {
      int i = 1193180000 / eax;
      io_out8(0x43, 0xb6);
      io_out8(0x42, i & 0xff);
      io_out8(0x42, i >> 8);
      i = io_in8(0x61);
      io_out8(0x61, (i | 0x03) & 0x0f);
    }
  }
  return 0;
}

int* inthandler0d(int *esp)
{
  struct TASK *task = task_now();
  struct CONSOLE *cons = task->cons;
  cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
  return &(task->tss.esp0);
}

int *inthandler0c(int *esp)
{
  struct TASK *task = task_now();
  struct CONSOLE *cons = task->cons;
  char s[30];
  cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
  sprintf(s, "EIP = %x/n", esp[11]);
  cons_putstr0(cons, s);
  return &(task->tss.esp0);
}
