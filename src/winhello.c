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

#define a 16807
#define m 2147483647
#define q (m / a)
#define r (m % a)

static long int seed = 1;
long int rand()
{
  long int hi = seed / q;
  long int lo = seed % q;
  long int test = a * lo - r * hi;
  if (test > 0)
    seed = test;
  else
    seed = test + m;
  return seed;
}

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
  api_closewin(win); 
  api_end();
}
