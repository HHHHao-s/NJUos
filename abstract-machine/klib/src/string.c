#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len=0;
  const char *p = s;
  for(;*p++;){len++;}
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *p=dst;
  for(;*src;src++){
    *dst++ = *src;
  }
  *dst = '\0';
  return p;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *p=dst;
  int i=0;
  for(;*src && i<n;){
    *p++ = *src++;
    i++;
  }
  if(i==n) dst[n-1] = '\0';
  else *p = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *old = dst;
  for(;*dst;dst++);
  for(;*src;){
    *dst++=*src++;
  }
  *dst = '\0';
  return old;
}

int strcmp(const char *s1, const char *s2) {
  
  for(;*s2 && *s1 && *s1==*s2;s1++,s2++);
  if(!*s1 && !*s2){
    return 0;
  }else{
    return *s1-*s2;
  }
}

int strncmp(const char *s1, const char *s2, size_t n) {
  
  for(;*s2 && *s1 && *s1++==*s2++ && n-->0;);
  if(!*s1 && !*s2){
    return 0;
  }else{
    return *s1-*s2;
  }
}

void *memset(void *s, int c, size_t n) {
  char *p = s;
  char cc = (char)(c%256);
  for(size_t i=0;i<n;i++){
    *p++ = cc;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  uint8_t buf[n];
  memcpy(buf, src, n);
  memcpy(dst, buf, n);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  uint8_t *pout=out;
  const uint8_t* pin = in;
  for(size_t i=0;i<n;i++){
    *pout++ = *pin++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
