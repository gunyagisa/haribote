#pragma once

#include "memory.h" 
#include "sheet.h"
#include "fifo.h"
#include "graphic.h"
#include "dsctbl.h"

#define BOOTINFO_ADDR 	0x00000ff0
#define DISKIMG_ADDR    0x00100000

#define EFLAGS_AC_BIT           0x00040000
#define CR0_CACHE_DISABLE       0x60000000

#define MEMMAN_ADDR     0x003c0000

#define MAX_TIMER       500

// nasmfunc.asm
void io_hlt(void);
void io_cli(void), io_sti(void), io_stihlt(void);
void io_out8(int port, int data), io_out16(int port, int data), io_out32(int port, int data);
int io_load_eflags(void), io_in8(int port), io_in16(int port), io_in32(int port),load_cr0();
void io_store_eflags(int eflags), store_cr0(int cr0);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void load_tr(int tr);
void farjmp(int, int), farcall(int,int);
int *inthandler0c_asm(int *esp);
int *inthandler0d_asm(int *esp);
void inthandler21_asm(void);
void inthandler2c_asm(void);
void inthandler20_asm(void);
void cons_putchar_asm(void);
void hrb_api_asm(void);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
void end_app_asm();
void api_end(void);

//struct to store the boot information written in asmhead.asm
typedef struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
} BOOTINFO;

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal);

// timer
struct TIMER {
  struct TIMER *next;
  unsigned int timeout;
  char flags, flags2;
  struct FIFO32 *fifo;
  int data;
};

struct TIMERCTL {
  unsigned int count, next;
  struct TIMER *t0;
  struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timectl;
extern struct TIMER *task_timer;

void init_pit(void);
void settimer(struct TIMER *timer, unsigned int timeout);
struct TIMER *timer_alloc(void);
void timer_init(struct TIMER *timer, FIFO32 *fifo, int data);
void timer_free(struct TIMER *timer);
int timer_cancel(struct TIMER *timer);
void timer_cancelall(struct FIFO32 *fifo);

//mtask
#define MAX_TASKS       1000
#define TASK_GDT0       3
#define MAX_TASKS_LV    100
#define MAX_TASKLEVELS  10

struct TSS32 {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
  int sel, flags;
  int level, priority;
  struct TSS32 tss;
  struct SEGMENT_DESCRIPTOR ldt[2];
  struct FIFO32 fifo;
  struct CONSOLE *cons;
  int ds_base, cons_stack;
};

struct TASKLEVEL {
  int running;
  int now;
  struct TASK *tasks[MAX_TASKS_LV];
};

struct TASKCTL {
  int now_lv;
  char lv_change;
  struct TASKLEVEL level[MAX_TASKLEVELS];
  struct TASK tasks0[MAX_TASKS];
};

extern struct TASKCTL *taskctl;

struct TASK * task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run (struct TASK *task, int , int);
void task_switch(void);
void task_sleep(struct TASK *task);
void task_switchsub(void);
void task_remove(struct TASK *);
void task_add(struct TASK *);
struct TASK * task_now(void);

// window.c
void make_wtitle8(unsigned char *, int, char *, char);
void make_window8(unsigned char *, int, int , char *, char);
void make_textbox8(struct SHEET *, int, int ,int , int, int);
void str_renderer_sht(struct SHEET *, int, int , int, int, char *, int);
extern void change_wtitle8(struct SHEET *sht, char act);

// console.c
struct CONSOLE {
  struct SHEET *sht;
  int cur_x, cur_y, cur_c;
  struct TIMER *timer;
};
void console_task(struct SHEET *, unsigned int); 
void cons_newline(struct CONSOLE *); 
void cons_runcmd(char *, struct CONSOLE *, int *, unsigned int);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal);
void cmd_clear(struct CONSOLE *cons);
void cmd_ls(struct CONSOLE *cons);
void cmd_cat(struct CONSOLE *cons, int *fat, char *cmdline);
void cmd_exit(struct CONSOLE *cons, int *fat);
void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal);
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);
void cons_putstr0(struct CONSOLE *cons, char *s);
void cons_putstr1(struct CONSOLE * cons,char *s, int n);

struct SHEET * open_console(struct SHTCTL *shtctl, int memtotal);

// file.c
struct FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
};
void file_readfat(int *, unsigned char *);
void file_loadfile(int ,int ,char * , int *, char *);
struct FILEINFO * file_search(char *cmdline, struct FILEINFO *finfo, int max);

// myfunc.c
void str_reverse(char *, int);
int decimal(int , char *, int , int);
int hex(unsigned int, char *, int, int );
int string (const char *, char *, int, int );
int vsprintf(char *, const char *, __builtin_va_list);
int sprintf(char *, const char *, ...);
unsigned int strlen(const char *);
int strncmp(const char *, const char *, unsigned int);
int strcmp(const char *, const char *);

