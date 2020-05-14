void api_putchar(int c);
void api_putstr0(char *s);
void apit_putstr1(char *s, int l);
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
int alloc_timer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);
void api_freetimer(int timer);
void api_beep (int tone);
int api_fopen(char *fname);
void api_fclose(int fhandle);
void api_fseek(int handle, int offset, int mode);
int api_fsize(int fhandle, int mode);
int api_fread(char *buf, int maxsize, int fhandle);
int api_cmdline(char *buf, int maxsize);
int api_getlang(void);
