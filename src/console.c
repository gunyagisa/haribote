#include "bootpack.h"
#include "dsctbl.h"

void console_task(struct SHEET *sht, unsigned int memtotal)
{
  struct TIMER *timer;
  struct TASK *task = task_now();
  int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
  char s[30], cmdline[30], *p;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct FILEINFO *finfo = (struct FILEINFO *) (DISKIMG_ADDR + 0x002600);
  int *fat = (int *) memman_alloc_4k(memman, 4 * 2800);
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) GDT_ADDR;

  file_readfat(fat, (unsigned char *) (DISKIMG_ADDR + 0x000200));
  
  fifo32_init(&task->fifo, 128, fifobuf, task);

  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  settimer(timer, 50);

  str_renderer_sht(sht, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

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
          if (cursor_c >= 0) {
            cursor_c = COL8_FFFFFF;
          }
        } else {
          timer_init(timer, &task->fifo, 1);
          if (cursor_c >= 0) {
            cursor_c = COL8_000000;
          }
        }
        settimer(timer, 50);
      }
      if (i == 2) { // cursor on
        cursor_c = COL8_FFFFFF;
      }
      if (i == 3) { // cursor off
        boxfill8(sht->buf, sht->bxsize, COL8_000000, cursor_x, 28, cursor_x + 7, 43);
        cursor_c = -1;
      }
      if (256 <= i && i <= 511) {
        if (i == 8 + 256) { // backspace
          if (cursor_x > 16) {
            str_renderer_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
            cursor_x -= 8;
          }
        } else if (i == 10 + 256) {
          str_renderer_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
          cmdline[cursor_x / 8 - 2] = 0;
          cursor_y = cons_newline(cursor_y, sht);
          if (strcmp(cmdline, "mem") == 0) {
            sprintf(s, "total %dMB", memtotal / (1024 * 1024));
            str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
            cursor_y = cons_newline(cursor_y, sht);
            sprintf(s, "free %dKB", memman_total(memman) / 1024);
            str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
            cursor_y = cons_newline(cursor_y, sht);
            cursor_y = cons_newline(cursor_y, sht);
          } else if (strcmp(cmdline, "cls") == 0) {
            for (int y = 28;y < 28 + 128;++y) {
              for (int x = 8;x < 8 + 240;x++) {
                sht->buf[x + y * sht->bxsize] = COL8_000000;
              }
            }
            sheet_refresh(sht, 8, 28, 8 + 240, 28 + 128);
            cursor_y = 28;
          } else if (strncmp(cmdline, "ls", 2) == 0 ) {
            for ( int x = 0; x < 224; ++x ) {
              if (finfo[x].name[0] == 0x0) break;
              if (finfo[x].name[0] != 0xe5) {
                if ((finfo[x].type & 0x18) == 0) {
                  sprintf(s, "filename.ext      %d", finfo[x].size);
                  for (int y = 0;y < 8;y++) {
                    s[y] = finfo[x].name[y];
                  }
                  s[9] = finfo[x].ext[0];
                  s[10] = finfo[x].ext[1];
                  s[11] = finfo[x].ext[2];
                  str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                  cursor_y = cons_newline(cursor_y, sht);
                }
              }
            }
          } else if (strncmp(cmdline, "cat ", 4) == 0) {
            int y;
            int x;
            for (y = 0;y < 11;++y) {
              s[y] = ' ';
            }
            y = 0;
            for (int x = 4; y < 11 && cmdline[x] != 0 ; x++) {
              if (cmdline[x] == '.' && y <= 8) {
                y = 8;
              } else {
                s[y] = cmdline[x];
                if ('a' <= s[y] && s[y] <= 'z') {
                  s[y] -= 0x20;
                }
                y++;
              }
            }
            for (x = 0; x < 224;) {
              if (finfo[x].name[0] == 0x00) {
                break;
              }
              if ((finfo[x].type & 0x18) == 0) {
                for (y = 0;y < 11;y++) {
                  if (finfo[x].name[y] != s[y]) {
                    goto type_next_file;
                  }
                }
                break;
              }
type_next_file:
              x++;
            }
            if (x < 224 && finfo[x].name[0] != 0x00) {
              y = finfo[x].size;
              p = (char *) memman_alloc_4k(memman, finfo[x].size);
              file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (DISKIMG_ADDR + 0x003e00));
              cursor_x = 8;
              for (y = 0; y < finfo[x].size; y++) {
                s[0] = p[y];
                s[1] = 0;
                if (s[0] == 0x09) { // tab
                  for (;;) {
                    str_renderer_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                    cursor_x += 8;
                    if (cursor_x == 8 + 240) {
                      cursor_x = 8;
                      cursor_y = cons_newline(cursor_y, sht);
                    }
                    if (((cursor_x - 8) & 0x1f) == 0) break;
                  }
                } else if (s[0] == 0x0a) { // kaigyou
                  cursor_x = 8;
                  cursor_y = cons_newline(cursor_y, sht);
                } else if (s[0] == 0x0d) { // carriage return
                  // nothing
                } else {
                  str_renderer_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                  cursor_x += 8;
                  if (cursor_x == 8 + 240) {
                    cursor_x = 8;
                    cursor_y = cons_newline(cursor_y, sht);
                  }
                }
              }
              memman_free_4k(memman, (int) p, finfo[x].size);
            } else {
                str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
                cursor_y = cons_newline(cursor_y, sht);
              }
              cursor_y = cons_newline(cursor_y, sht);
            } else if (strcmp(cmdline, "hlt") == 0) {
              for (int y = 0;y < 11;y++) {
                s[y] = ' ';
              }
              s[0] = 'H';
              s[1] = 'L';
              s[2] = 'T';
              s[8] = 'H';
              s[9] = 'R';
              s[10] = 'B';
              int x;
              for (x = 0; x < 224;) {
                if (finfo[x].name[0] == 0x00) {
                  break;
                }
                if ((finfo[x].type & 0x18) == 0) {
                  for (int y = 0; y < 11; y++) {
                    if (finfo[x].name[y] != s[y]) {
                      goto hlt_next_file;
                    }
                  }
                  break;
                }
hlt_next_file:
                x++;
              }
              if (x < 224 && finfo[x].name[0] != 0x00) {
                p = (char *) memman_alloc_4k(memman, finfo[x].size);
                file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (DISKIMG_ADDR + 0x003e00));
                set_sgmntdsc(gdt + 1003, finfo[x].size - 1, (int) p, AR_CODE32_ER);
                farjmp(0, 1003 * 8);
                memman_free_4k(memman, (int) p, finfo[x].size);
              } else {
                str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
                cursor_y = cons_newline(cursor_y, sht);
              }
              cursor_y = cons_newline(cursor_y, sht);
            }else if (cmdline[0] != 0) {
              str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
              cursor_y = cons_newline(cursor_y, sht);
              cursor_y = cons_newline(cursor_y, sht);
            }
            str_renderer_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
            cursor_x = 16;
        } else {
          if (cursor_x < 240) {
            s[0] = i - 256;
            s[1] = 0;
            cmdline[cursor_x / 8 - 2] = i - 256;
            str_renderer_sht(sht, cursor_x, cursor_y,  COL8_FFFFFF, COL8_000000, s, 1);
            cursor_x += 8;
          }
        }
      }
      if (cursor_c >= 0) {
        boxfill8(sht->buf, sht->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
      }
      sheet_refresh(sht, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
    }
  }
}


int cons_newline(int cursor_y, struct SHEET *sheet)
{
  if (cursor_y < 28 + 112) {
    cursor_y += 16;
  } else {
    for (int y = 28;y < 28 + 112;y++) {
      for (int x = 8;x < 8 + 240;x++) {
        sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
      }
    }
    for (int y = 28 + 112;y < 28 + 128;y++) {
      for (int x = 8; x < 8 + 240;x++) {
        sheet->buf[x + y * sheet->bxsize] = COL8_000000;
      }
    }
    sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
  }
  return cursor_y;
}
