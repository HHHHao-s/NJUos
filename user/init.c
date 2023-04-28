#include "ulib.h"



int main() {
  // Example:
  char *p = "test\n";
  
  for (size_t i = 0; i < 5; i++)
  {
    syscall(SYS_kputc,p[i], 0,0,0);
  }
    
  
  return 0;
}
