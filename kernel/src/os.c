#include <common.h>

static void os_init() {
  ioe_init();
  pmm->init();
}

static void os_run() {
  void * p=pmm->alloc(10000);
  pmm->free(p);
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
