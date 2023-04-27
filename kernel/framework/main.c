#include <kernel.h>
#include <klib.h>

int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  uproc->init();
  
  while(1);
  return 1;
}
