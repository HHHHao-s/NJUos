#include <kernel.h>
#include <klib.h>

int main() {
  ioe_init();
  cte_init(os->trap);
  dev->init();
  os->init();
  mpe_init(os->run);
  
  return 1;
}
