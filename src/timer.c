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
    for (int i = 0;i < MAX_TIMER;++i) {
        timectl.timer[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void)
{
    for (int i = 0;i < MAX_TIMER;++i) {
        if (timectl.timer[i].flags == 0) {
            timectl.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &timectl.timer[i];
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer)
{
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, FIFO8 *fifo, unsigned char data)
{
    timer->fifo = fifo;
    timer->data = data;
}

void inthandler20(int *esp)
{
    io_out8(PIC0_OCW2, 0x60);
    timectl.count++;
    
    if (timectl.next > timectl.count) return;

    timectl.next = 0xffffffff;
    for (int i = 0;i < MAX_TIMER;++i) {
        if (timectl.timer[i].timeout <= timectl.count) {
            timectl.timer[i].flags = TIMER_FLAGS_ALLOC;
            fifo8_put(timectl.timer[i].fifo, timectl.timer[i].data);
        } else {
            if (timectl.next > timectl.timer[i].timeout) {
                timectl.next = timectl.timer[i].timeout;
            }
        }
    }
}

void settimer(struct TIMER *timer, unsigned int timeout)
{
    timer->timeout = timeout + timectl.count;
    timer->flags = TIMER_FLAGS_USING;
    if (timectl.next > timer->timeout)
        timectl.next = timer->timeout;
}
