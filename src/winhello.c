int api_openwin(char *buf, int xsize, int ysize, int col_inv, char*title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfillwin(int win, int , int , int ,int , int );
void api_end(void);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);

void HariMain()
{
  int win;
  api_initmalloc();
  char *buf = api_malloc(150 * 50);
  win = api_openwin(buf, 150, 50, -1, "hello");
  api_boxfillwin(win, 8, 36, 141, 43, 3);
  api_putstrwin(win, 28, 28, 0, 12, "hello world");
  api_end();
}
