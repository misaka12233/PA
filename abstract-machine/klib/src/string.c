#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len;
  for (len = 0; s[len] != '\0'; len++);
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i = -1;
  do
  {
    i++;
    dst[i] = src[i];
  }
  while (src[i] != '\0');
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  for (int i = 0; i < n; i++)
    dst[i] = src[i];
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t l1 = strlen(dst), l2 = strlen(src);
  for (size_t i = 0; i <= l2; i++)
    dst[l1 + i] = src[i];
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  for(size_t i = 0; s1[i] != '\0' || s2[i] != '\0'; i++)
    if (s1[i] != s2[i])
      return s1[i] - s2[i];
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for(int i = 0; i < n; i++)
    if (s1[i] != s2[i])
      return s1[i] - s2[i];
  return 0;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *st = s;
  for (int i = 0; i < n; i++)
    st[i] = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  return strncpy(out, in, n);
}

int memcmp(const void *s1, const void *s2, size_t n) {
  return strncmp(s1, s2, n);
}

#endif
