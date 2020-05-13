#include "applib.h"
#include "myfunc.h"

#define MAX     1000

void HariMain(void)
{
  char flag[MAX], s[8];

  for (int i = 0; i < MAX; i++) {
    flag[i] = 0;
  }

  for (int i = 2; i < MAX; i++) {
    if (flag[i] == 0) {
      sprintf(s, "%d ", i);
      api_putstr0(s);
      for (int j = i * 2; j < MAX; j += i) {
        flag[j] = 1;
      }
    }
  }
  api_end();
}

