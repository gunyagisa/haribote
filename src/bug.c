void api_putchar(int c);
void api_end(void);

void HariMain()
{
  char s[100];
  s[10] = 'A';
  api_putchar(s[10]);
  s[102] = 'B';
  api_putchar(s[102]);
  s[123] = 'C';
  api_putchar(s[123]);

  api_end();
}

