#include <common.h>
#define KMT_STACK_SIZE 8192
#define KMT_FENCE_SIZE 32
#define KMT_NAME_SIZE 32
#define KMT_LOG_SIZE 32

int task_list_insert(task_t *task);
void task_list_delete(task_t *task);
task_t* task_alloc();
void log_map(task_t *task, void *vaddr, void *paddr, int prot);


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
  
  union
  {  
    struct{
      int id;
      enum {
      RUNNING = 1,
      RUNNABLE,
      ZOMBIE,
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
      struct task *parent;
      int exit_status;
      struct{
        struct {uintptr_t va, pa;} log[KMT_LOG_SIZE]; // 记录map过的pa
        int log_len;
      };
      
      uint8_t fence[KMT_FENCE_SIZE]; // 用来防止overflow 
    };
    
    uint8_t stack[KMT_STACK_SIZE]; // 栈指针
  };
};// 整个结构体就是KMT_STACK_SIZE


struct
{
  task_t *head;
  spinlock_t lock;
  sem_t having;
}task_list;



spinlock_t print_lock;

