#include "ulib.h"



int main() {
  // Example:
  // char *pfar = "far\n";
  // char *pson =  "son\n";
  int id;
  if((id=syscall(SYS_fork,0,0,0,0))==0){ // son
    syscall(SYS_kputc, id+'0',0,0,0);
  }else{
    syscall(SYS_kputc, id+'0',0,0,0);
  }
  
    
  while(1);
  return 0;
}
