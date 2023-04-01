#include <common.h>

static void os_init() {
  ioe_init();
  pmm->init();
}

static void os_run() {
  void * p=pmm->alloc(1000);
  printf("66666%p", p);
  pmm->free(p);
  
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
