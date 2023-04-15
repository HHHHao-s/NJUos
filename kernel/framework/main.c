#include <kernel.h>
#include <klib.h>

int main() {
  ioe_init();
  char buf[4];

  snprintf(buf, 4, "%d", 123456);
  printf(buf);
  cte_init(os->trap);
  dev->init();
  os->init();
  mpe_init(os->run);
  
  return 1;
}
