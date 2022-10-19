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
  size_t len = strlen(src);
  for (int i = 0; i <= len; i++)
    dst[i] = src[i];
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
  size_t len1 = strlen(s1), len2 = strlen(s2);
  for(size_t i = 0; i < len1 || i < len2; i++)
    if (s1[i] != s2[i])
      return (int)s1[i] - s2[i];
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
  char tmp[n];
  strncpy(tmp, src, n);
  return strncpy(dst, tmp, n);
}

void *memcpy(void *out, const void *in, size_t n) {
  return strncpy(out, in, n);
}

int memcmp(const void *s1, const void *s2, size_t n) {
  return strncmp(s1, s2, n);
}

#endif
