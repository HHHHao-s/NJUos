#include <common.h>


void test(){
  printf("niubi%d:%s\n", 1000,"cennn");
}

static void os_init() {
  test();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};



