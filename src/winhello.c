int api_openwin(char *buf, int xsize, int ysize, int col_inv, char*title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfillwin(int win, int , int , int ,int , int );
void api_end(void);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0,int y0, int x1, int y1);

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
  char *buf = api_malloc(150 * 100);
  win = api_openwin(buf, 150, 100, -1, "hello");
  api_boxfillwin(win, 6, 26, 143, 93, 0);
  int x, y;
  for (int i = 0; i < 1000; i++) {
    x = (rand() % 137) + 6;
    y = (rand() % 67) + 26;
    api_point(win, x, y, 3);
  }
  api_refreshwin(win, 6, 26, 144, 94);
  api_end();
}
