#include <kernel.h>
#include <klib.h>

int main() {
  ioe_init();
  pmm->init();
  cte_init(os->trap);
  os->init();
  kmt->init();
  uproc->init();
  iset(true);// 开中断
  yield();
  while(1);
  return 1;
}
