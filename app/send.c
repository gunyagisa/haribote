#include "applib.h"

void HariMain()
{
  char cmdline[48], *p;

  api_cmdline(cmdline, 48);

  api_write_serial(cmdline);
  api_write_serial("\n");

  api_end();
}
