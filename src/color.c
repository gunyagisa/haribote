int api_openwin(char *buf, int xsize, int ysize, int col_inv, char*title);
void api_closewin(int win);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfillwin(int win, int , int , int ,int , int );
void api_end(void);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0,int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
int api_getkey(int mode);

unsigned char rgb2pal(int r, int g, int b, int x, int y);

void HariMain()
{
  char *buf;
  int win;
  api_initmalloc();
  buf = api_malloc(144 * 164);
  win = api_openwin(buf, 144, 164, -1, "color");
  
  for (int y = 0; y < 128; y++) {
    for (int  x = 0; x < 128; x++) {
      buf[(x + 8) + (y + 28) * 144] = rgb2pal(x * 2, y * 2, 0, x , y);
    }
  }
  api_refreshwin(win, 8, 28, 136, 156);
  api_getkey(1);
  api_end();
}

unsigned char rgb2pal(int r, int g, int b, int x, int y)
{
  static int table[4] = {3, 1, 0, 2};

  x &= 1;
  y &= 1;
  int i = table[x + y * 2];
  r = (r * 21) / 256;
  g = (g * 21) / 256;
  b = (b * 21) / 256;
  r = (r + i) / 4;
  g = (g + i) / 4;
  b = (b + i) / 4;
  return 16 + r + g * 6 + b * 36;
}
