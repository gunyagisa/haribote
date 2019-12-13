#include "bootpack.h"
#include "keyboard.h"

void wait_KBC_sendready(void)
{
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & 0x02) == 0)
            break;
    }
}

void init_keyboard(void)
{
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDATA, KBC_MODE_MOUSE);
}
