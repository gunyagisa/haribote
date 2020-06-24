#include "applib.h"

void HariMain()
{
  char cmdline[48], *p;

  api_cmdline(cmdline, 48);

  for (p = cmdline; *p != 0; ++p) {
    api_write_serial(*p);
  }

  api_end();
}
