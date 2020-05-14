#include <stdarg.h>

int string(const char *, char *, int ,int);

void str_reverse(char *str, int size)
{
  for (int j = 0;j < size / 2;++j) {
    char tmp = str[size - 1 - j];
    str[size - 1 - j] = str[j]; str[j] = tmp;
  }
}

int decimal(int n, char *str, int zero, int width)
{
  static char buf[13] = {0};
  int d = 10;
  int i = 0;
  if (n == 0) buf[i++] = '0';
  while (n != 0) {
    buf[i++] = n % d + '0';
    n /= d;
  }
  str_reverse(buf, i);
  string(buf, str, zero, width);
  return i;
}

int hex(unsigned int n, char *str, int zero, int width)
{
  static char buf[10] = {0};
  int d = 16;
  int i = 0;
  if (n == 0) buf[i++] = '0';
  while (n != 0) {
    int tmp = n % d;
    if (tmp < 10) {
      buf[i++] = '0' + tmp;
    } else {
      buf[i++] = 'a' + (tmp - 10);
    }
    n /= d;
  }

  str_reverse(buf, i);
  string(buf, str, zero, width);

  return i;
}

int string(const char *n, char *str, int zero, int width)
{
  int i = 0;
  while (*n != '\0') {
    *(str++) = *(n++);
    i++;
  }
  return i;
}

int vsprintf(char *str, const char *fmt, va_list arg)
{
  int len = 0;
  int size = 0;
  int zeroflag, width;

  while (*fmt != '\0') {
    if (*fmt == '%') {
      zeroflag = width = 0;
      fmt++;
      if (*fmt == '0') {
        zeroflag = 1;
        fmt++;
      }
      if ((*fmt >= '0') && (*fmt <= 9)) {
        width = *(fmt++) - '0';
      }
      switch (*fmt) {
        case 'd':
          size = decimal(va_arg(arg, int),str,zeroflag, width);
          break;
        case 'x':
          size = hex(va_arg(arg, int), str, zeroflag, width);
          break;
        case 's':
          size = string(va_arg(arg, char *), str, zeroflag, width);
          break;
        default:
          *(str) = *(fmt);
          size = 1;
          break;
      }
      fmt++;
      str += size;
      len += size;
    } else {
      *(str++) = *(fmt++);
      len++;
    }
  }

  *(str) = '\0';
  va_end(arg);
  return len;
}


int sprintf(char *str, const char *fmt, ...)
{
  va_list arg;
  int len;

  va_start(arg, fmt);
  len = vsprintf(str, fmt, arg);

  va_end(arg);

  return len;
}

unsigned int strlen(const char *s)
{
  int i = 0;
  while (*s != 0) {
    ++i;
    ++s;
  }
  return i;
}


int strncmp(const char *s1, const char *s2, unsigned int n)
{
  for (int i = 0; i < n; ++i) {
    if (*s1 == *s2) {
      s1++;
      s2++;
    } else {
      return *s1 - *s2;
    }
  }
  return 0;
}
int strcmp(const char *s1, const char *s2)
{
  return strncmp(s1, s2, strlen(s2));
}

#define a 16807
#define m 2147483647
#define q (m / a)
#define r (m % a)
long int rand()
{
  int seed = 1;
  long int hi = seed / q;
  long int lo = seed % q;
  long int test = a * lo - r * hi;
  if (test > 0)
    seed = test;
  else
    seed = test + m;
  return seed;
}

int memcmp(const void *buf1, const void *buf2, unsigned int n)
{
  for (int i = 0; i < n; i++) {
    if (((unsigned char *)buf1)[i] != ((unsigned char *)buf2)[i]) 
      return ((unsigned char * )buf1)[i] - ((unsigned char *)buf2)[i];
  }
  return 0;
}

long strtol(char *s, char **t, int n)
{
}
