#include <common.h>



struct task {
  int id;
  enum {
    RUNNING = 1,
    RUNNABLE,
    DEAD,
  }status;

  void *entry;
  struct task *next;
  Context *context;
  uint8_t stack[8192];
  // uint8_t bird[32]; // 用来防止overflow
};



struct spinlock {
  
  int lock;// 锁
  const char *name;
};

struct semaphore {
  struct spinlock bin_lock; // 自旋锁
  int val;// 量
  const char *name; 
};
