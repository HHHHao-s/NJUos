#include <kernel.h>
#include <klib.h>

int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  // mpe_init(os->run);
  while (1);
  return 1;
}
