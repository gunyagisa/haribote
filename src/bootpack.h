#pragma once

#include "memory.h"

#define BOOTINFO_ADDR 	0x00000ff0

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

struct TSS32 {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
  int sel, flags;
  struct TSS32 tss;
};

struct TASKCTL {
  int running;
  int now;
  struct TASK *tasks[MAX_TASKS];
  struct TASK tasks0[MAX_TASKS];
};
struct TASK * task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run (struct TASK *task);
void task_switch(void);
void task_sleep(struct TASK *task);
