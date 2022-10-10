#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

void sprint_int(char *out, size_t *len, int val)
{
  if (val == 0) return;
  sprint_int(out, len, val / 10);
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
      switch (fmt[i])
      {
        case 'd':
          int x = va_arg(ap, int);
          if (x != 0)
            sprint_int(out, &j, x);
          else
          {
            out[j] = '0';
            j++;
          }
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
