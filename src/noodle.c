#include "myfunc.h"

int api_openwin(char *buf, int xsize, int ysize, int col_inv, char*title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfillwin(int win, int , int , int ,int , int );
void api_end(void);
void api_initmalloc(void);
char *api_malloc(int size);
int api_getkey(int mode);
int api_alloctimer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);

void HariMain()
{
  char *buf, s[12];
  int win, timer, sec = 0, min = 0, hou = 0;

  api_initmalloc();
  buf = api_malloc(150 * 50);
  win = api_openwin(buf, 150,  50, -1, "noodle");
  timer = api_alloctimer();
  api_inittimer(timer, 128);
  api_putstrwin(win ,28, 27, 0, 11, s);

  for (;;) {
    sprintf(s, "%d:%d:%d", hou, min, sec);
    api_boxfillwin(win, 28, 27, 115, 41, 7);
    api_putstrwin(win, 28, 27, 0, 11, s);
    api_settimer(timer, 100);
    sec++;
    if (api_getkey(1) != 128) break;
    if (sec == 60) {
      min++;
      sec = 0;
      if (min == 60) {
        hou++;
        min = 0;
      }
    }
  }
  api_end();
}
