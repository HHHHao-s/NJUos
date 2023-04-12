#include <common.h>



struct task {
  // TODO
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
  // TODO
};

struct semaphore {
  // TODO
};