#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}


char* itoa(char *a, int i){// return strlen
  char *ret= a;
  char buf[1024];
  char *p = buf;
  int count = 0;
  int n=0;
  if(i==0){
    *a++='0';
  }else if(i>0){
    while(i>0){
    n=i%10;
    *p++ = n+'0';
    i/=10;
    count++;
    }
    for(;p-->buf;){
      *a++ = *p;
    }
  }else{
    i=-i;
    if(i<0){ // INT_MIN
      for(p="-214748364";*p;){
        *a++ = *p++;
      }
    }else{
      while(i>0){
        n=i%10;
        *p++ = n+'0';
        i/=10;
        count++;
      }
      *p++ = '-';
      for(;p-->buf;){
        *a++ = *p;
      }
    }
  }
  
  *a = '\0';
  return ret;
}


int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
