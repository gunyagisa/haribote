
int memcmp(const void *buf1, const void *buf2, unsigned int n) {
  for (int i = 0; i < n; i++) {
    if (((unsigned char *)buf1)[i] != ((unsigned char *)buf2)[i])
      return ((unsigned char *)buf1)[i] - ((unsigned char *)buf2)[i];
  }
  return 0;
}

long strtol(char *p, char **endptr, int base) {
    int j, k;
    for (j = 0; '0' <= p[j] && p[j] <= '9'; j++) {}
    int i = 0;
    k = j;
    j--;
    for (int n = 1; j >= 0; j--, n *= 10) {
      i += (p[j] - '0') * n;
    }
    *endptr = p + k;
    return i;
}
