#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

struct task {
  // int id;
  union
  {  
    struct{
      enum {
      RUNNING = 1,
      RUNNABLE,
      DEAD,
      CAN_BE_CLEAR
      }status;
      int a;
      int b;
      int c;
      unsigned char fence[32]; // 用来防止overflow 
    };
    
    unsigned char stack[8192]; // 栈指针
  };
};// 整个结构体就是2个page

int main(int args, char **argv)
{
    struct task *t=malloc(sizeof(struct task));

    printf("&task->fence + 1 - task = %ld\n", (void *)(&t->fence + 1) - (void *)t);

    return 0;
}