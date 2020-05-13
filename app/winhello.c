#include "applib.h"

void HariMain()
{
  int win;
  api_initmalloc();
  char *buf = api_malloc(160 * 100);
  win = api_openwin(buf, 160 , 100, -1, "new line");
  for (int i = 0; i < 8; i++) {
    api_linewin(win + 1, 8, 26, 77, i * 9 + 26, i);
    api_linewin(win + 1, 88, 26, i * 9 + 88, 89, i);
  }
  api_refreshwin(win, 6, 26, 154, 90);
  for (;;) {
    if (api_getkey(1) == 0x0a) break;
  }
  api_closewin(win); 
  api_end();
}
