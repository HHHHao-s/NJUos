#include "ulib.h"



int main() {
  // Example:
  char *p = "test";
  while(1){
    for (size_t i = 0; i < 4; i++)
    {
      syscall(SYS_kputc,p[i], 0,0,0);
    }
    
  }
  return 0;
}
