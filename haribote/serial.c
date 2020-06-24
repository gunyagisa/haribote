#include "bootpack.h"
#define PORT 0x3f8

void serial_init()
{
  io_out8(PORT+1, 0x00);
  io_out8(PORT+3, 0x80);
  io_out8(PORT+0, 0x03);
  io_out8(PORT+1, 0x00);
  io_out8(PORT+3, 0x03);
  io_out8(PORT+2, 0xc7);
  io_out8(PORT+4, 0x0b);
}

int is_serial_empty()
{
  return io_in8(PORT + 5) & 0x20;
}

void write_serial(char a)
{
  while (is_serial_empty() == 0);

  io_out8(PORT, a);
}

void write_serial_str(char *str)
{
  while (*str != 0) {
    write_serial(*str);
    str++;
  }
}
