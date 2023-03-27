#include <kernel.h>
#include <klib.h>

int main() {
  os->init();
  mpe_init(os->run);
  void *p=pmm->alloc(1000);
  free(p);
  return 1;
}
