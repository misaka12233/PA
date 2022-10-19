#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char out[1024];
  int return_val = vsprintf(out, fmt, ap);
  va_end(ap);
  for (int i = 0; i < return_val; i++)
    putch(out[i]);
  return return_val;
}

void sprint_uint(char *out, size_t *len, unsigned int val)
{
  if (val == 0) return;
  sprint_uint(out, len, val / 10);
  out[*len] = val % 10 + '0';
  (*len)++;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  size_t i, j;
  i = j = 0;
  out[j] = '\0';
  while (fmt[i] != '\0')
  {
    if (fmt[i] == '%')
    {
      i++;
      int length = 0;
      while ('0' <= fmt[i] && fmt[i] <= '9')
      {
        length = length * 10 + fmt[i] - '0';
        i++;
      }
      switch (fmt[i])
      {
        case 'd':
          int st, ed;
          st = j;
          int x = va_arg(ap, int);
          unsigned int x_abs = x;
          if (x < 0)
          {
            out[j] = '-';
            j++;
            x_abs = -x_abs;
          }
          if (x != 0)
            sprint_uint(out, &j, x_abs);
          else
          {
            out[j] = '0';
            j++;
          }
          ed = j;
          if (st - ed + 1 < length)
          {
            int new_st = st + length - 1 + st - ed;
            for (int i = st + length - 1; i >= new_st; i--)
              out[i] = out[st + i - new_st];
            for (int i = st; i < new_st; i++)
              out[i] = '0';
          }
          j = length ? st + length : j;
          break;
        case 's':
          char *s = va_arg(ap, char *);
          j += strlen(s);
          strcat(out, s);
          break;
        default:
          return -1;
          break;
      }
    }
    else
    {
      out[j] = fmt[i];
      j++;
    }
    i++;
    out[j] = '\0';
  }
  return j;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int return_val = vsprintf(out, fmt, ap);
  va_end(ap);
  return return_val;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
