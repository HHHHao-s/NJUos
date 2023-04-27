#include <am.h>
#include <syscall.h>
// #include <os.h>
#define MODULE(mod) \
  typedef struct mod_##mod##_t mod_##mod##_t; \
  extern mod_##mod##_t *mod; \
  struct mod_##mod##_t

#define MODULE_DEF(mod) \
  extern mod_##mod##_t __##mod##_obj; \
  mod_##mod##_t *mod = &__##mod##_obj; \
  mod_##mod##_t __##mod##_obj

#define MAX_CPU 8
#define MAX_TASKS 32

#define current_task currents[cpu_current()]
#define P(s) kmt->sem_wait(s)
#define V(s) kmt->sem_signal(s)

typedef struct spinlock spinlock_t;
typedef struct semaphore sem_t;
typedef struct task task_t;

#define atom_printf(...)  \
  while(!atomic_xchg(&print_lock.lock,0));\
  printf(__VA_ARGS__);\
  atomic_xchg(&print_lock.lock,1)

task_t *currents[MAX_CPU];



typedef Context *(*handler_t)(Event, Context *);

typedef struct _handler_node{
  int seq,event;
  handler_t handler;
  struct _handler_node *next;
}handler_node;// handler的链表，按照seq排序

handler_node * handler_head;



MODULE(os) {
  void (*init)();
  void (*run)();
  Context *(*trap)(Event ev, Context *context);
  void (*on_irq)(int seq, int event, handler_t handler);
};

MODULE(pmm) {
  void  (*init)();
  void *(*alloc)(int size);
  void  (*free)(void *ptr);
};




MODULE(kmt) {
  void (*init)();
  int  (*create)(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
  void (*teardown)(task_t *task);
  void (*spin_init)(spinlock_t *lk, const char *name);
  void (*spin_lock)(spinlock_t *lk);
  void (*spin_unlock)(spinlock_t *lk);
  void (*sem_init)(sem_t *sem, const char *name, int value);
  void (*sem_wait)(sem_t *sem);
  void (*sem_signal)(sem_t *sem);
};

typedef struct device device_t;
MODULE(dev) {
  void (*init)();
  device_t *(*lookup)(const char *name);
};

MODULE(uproc) {
  void (*init)();
  int (*kputc)(task_t *task, char ch);
  int (*fork)(task_t *task);
  int (*wait)(task_t *task, int *status);
  int (*exit)(task_t *task, int status);
  int (*kill)(task_t *task, int pid);
  void *(*mmap)(task_t *task, void *addr, int length, int prot, int flags);
  int (*getpid)(task_t *task);
  int (*sleep)(task_t *task, int seconds);
  int64_t (*uptime)(task_t *task);
};
