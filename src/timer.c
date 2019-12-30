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
    timectl.next = 0xffffffff;
    timectl.using = 0;
    for (int i = 0;i < MAX_TIMER;++i) {
        timectl.timers0[i].flags = 0;
    }
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
    io_out8(PIC0_OCW2, 0x60);
    timectl.count++;
    if (timectl.next > timectl.count) return;

    int i;
    for (i = 0;i < timectl.using;++i) {
        if (timectl.timers[i]->timeout > timectl.count) break;
        timectl.timers[i]->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timectl.timers[i]->fifo, timectl.timers[i]->data);
    }
    timectl.using -= i;
    for (int j = 0;j < timectl.using;++j) {
        timectl.timers[j] = timectl.timers[j + i];
    }
    if (timectl.using > 0)
        timectl.next = timectl.timers[0]->timeout;
    else
        timectl.next = 0xffffffff;
}

void settimer(struct TIMER *timer, unsigned int timeout)
{
    timer->timeout = timeout + timectl.count;
    timer->flags = TIMER_FLAGS_USING;
    int e = io_load_eflags();
    io_cli();

    int i;
    for (i = 0;i < timectl.using;++i) {
        if (timectl.timers[i]->timeout >= timer->timeout) break;
    }

    for (int j = timectl.using;j > i;j--) {
        timectl.timers[j] = timectl.timers[j - 1];
    }
    timectl.using++;
    timectl.timers[i] = timer;
    timectl.next = timectl.timers[0]->timeout;
    io_store_eflags(e);
}
