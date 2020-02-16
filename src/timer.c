#include "bootpack.h"
#include "interrupt.h"
#include "fifo.h"

#define PIT_CTRL        0x0043
#define PIT_CNT0        0x0040

#define TIMER_FLAGS_ALLOC       1
#define TIMER_FLAGS_USING       2
struct TIMERCTL timectl;

void init_pit(void)
{
  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);
  timectl.count = 0;
  for (int i = 0;i < MAX_TIMER;++i) {
    timectl.timers0[i].flags = 0;
  }
  struct TIMER *t = timer_alloc();
  t->timeout = 0xffffffff;
  t->flags = TIMER_FLAGS_USING;
  t->next = 0;
  timectl.t0 = t;
  timectl.next = 0xffffffff;
  timectl.using = 1;
}

struct TIMER *timer_alloc(void)
{
  for (int i = 0;i < MAX_TIMER;++i) {
    if (timectl.timers0[i].flags == 0) {
      timectl.timers0[i].flags = TIMER_FLAGS_ALLOC;
      return &timectl.timers0[i];
    }
  }
  return 0;
}

void timer_free(struct TIMER *timer)
{
  timer->flags = 0;
}

void timer_init(struct TIMER *timer, FIFO32 *fifo, int data)
{
  timer->fifo = fifo;
  timer->data = data;
}

void inthandler20(int *esp)
{
  // send finish
  io_out8(PIC0_OCW2, 0x60);
  timectl.count++;
  if (timectl.next > timectl.count) return;

  struct TIMER *timer = timectl.t0;
  for (;;) {
    if (timer->timeout > timectl.next) break;
    timer->flags = TIMER_FLAGS_ALLOC;
    fifo32_put(timer->fifo, timer->data);
    timer = timer->next;
  }
  timectl.t0 = timer;
  timectl.next = timer->timeout;
}


void settimer(struct TIMER *timer, unsigned int timeout)
{
  int e;
  struct TIMER *t, *s;
  timer->timeout = timeout + timectl.count;
  timer->flags = TIMER_FLAGS_USING;
  e = io_load_eflags();
  io_cli();
  timectl.using++;
  t = timectl.t0;
  if (timer->timeout <= t->timeout) {
    timectl.t0 = timer;
    timer->next = t;
    timectl.next = timer->timeout;
    io_store_eflags(e);
    return;
  }

  for (;;) {
    s = t;
    t = t->next;
    if (timer->timeout <= t->timeout) {
      s->next = timer;
      timer->next = t;
      io_store_eflags(e);
      return;
    }
  }
}
