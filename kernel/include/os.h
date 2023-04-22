#include <common.h>
#define KMT_STACK_SIZE 8192*4
#define KMT_FENCE_SIZE 32
#define KMT_NAME_SIZE 32


int task_list_insert(task_t *task);
void task_list_delete(task_t *task);

struct spinlock {
  
  int lock;// 锁
  char name[KMT_NAME_SIZE];
};

struct semaphore {
  struct spinlock bin_lock; // 自旋锁
  int val;// 量
  char name[KMT_NAME_SIZE];
};


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
      struct spinlock lock;
      char name[KMT_NAME_SIZE];
      void *entry;
      void *arg;
      struct task *next;
      Context *context;
      AddrSpace as;
      uint8_t fence[KMT_FENCE_SIZE]; // 用来防止overflow 
    };
    
    uint8_t stack[KMT_STACK_SIZE]; // 栈指针
  };
};// 整个结构体就是2个page


struct
{
  task_t *head;
  spinlock_t lock;
  sem_t having;
}task_list;



spinlock_t print_lock;

