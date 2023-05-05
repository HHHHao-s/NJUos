#include "ulib.h"



int main() {
  // Example:
  // char *pfar = "far\n";
  // char *pson =  "son\n";
  volatile int a[12] = {-1,-1};
  int id;
  

  for(int i=0;i<3;i++){
    if((id=syscall(SYS_fork,0,0,0,0))==0){ // son
      a[0] = i;
      while(1){
        syscall(SYS_kputc, a[0]+'0',0,0,0);
        for(int j=0;j<100000;j++); // spin
        }    
    }
  }


  // if((id=syscall(SYS_fork,0,0,0,0))==0){ // son
  //   a[0] = 's';
  //   while(1){
  //     syscall(SYS_kputc, a[0],0,0,0);
  //     for(int j=0;j<100000;j++); // spin
  //     }
  //   }else{
  //     a[0] = 'f';
  //     while(1){
  //       syscall(SYS_kputc, a[0],0,0,0);
  //       for(int j=0;j<100000;j++); // spin
  //     }
  //   }
      
  
  
    
  while(1);
  return 0;
}
