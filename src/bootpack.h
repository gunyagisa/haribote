#pragma once

#include "memory.h"
#include "sheet.h"

#define BOOTINFO_ADDR 	0x00000ff0
#define DISKIMG_ADDR    0x00100000

#define EFLAGS_AC_BIT           0x00040000
#define CR0_CACHE_DISABLE       0x60000000

#define MEMMAN_ADDR     0x003c0000

#define MAX_TIMER       500

#include "fifo.h"
// nasmfunc.asm
void io_hlt(void);
void io_cli(void), io_sti(void), io_stihlt(void);
void io_out8(int port, int data), io_out16(int port, int data), io_out32(int port, int data);
int io_load_eflags(void), io_in8(int port), io_in16(int port), io_in32(int port),load_cr0();
void io_store_eflags(int eflags), store_cr0(int cr0);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void load_tr(int tr);
void farjmp(int, int);
void inthandler21_asm(void);
void inthandler2c_asm(void);
void inthandler20_asm(void);

//struct to store the boot information written in asmhead.asm
typedef struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
} BOOTINFO;

// timer
extern struct TIMER *task_timer;
struct TIMER {
  struct TIMER *next;
  unsigned int timeout, flags;
  FIFO32 *fifo;
  unsigned char data;
};

struct TIMERCTL {
  unsigned int count, next, using;
  struct TIMER timers0[MAX_TIMER];
  struct TIMER *t0;
};
extern struct TIMERCTL timectl;

void init_pit(void);
void settimer(struct TIMER *timer, unsigned int timeout);
struct TIMER *timer_alloc(void);
void timer_init(struct TIMER *timer, FIFO32 *fifo, int data);

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
  struct FIFO32 fifo;
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

// console.c
void console_task(struct SHEET *, unsigned int); 
int cons_newline(int , struct SHEET *);

