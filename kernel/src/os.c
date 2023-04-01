#include <common.h>

static void os_init() {
  ioe_init();
  pmm->init();
}

static void os_run() {
  while (1);
  
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
