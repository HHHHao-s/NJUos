// #include <common.h>
#include <os.h>
#include <devices.h>
#include <hashmap.h>

static void os_run();
static Context * os_trap(Event ev, Context *context);
static void os_irq(int seq, int event, handler_t handler);
static Context *inter_handler(Event ev, Context *ctx);
static Context *yield_handler(Event ev, Context *ctx);
static Context *page_handler(Event ev, Context *ctx);
static Context *iodev_handler(Event ev, Context *ctx);
static Context *syscall_handler(Event ev, Context *ctx);
static Context *error_handler(Event ev, Context *ctx);

struct{
  sem_t last,having;
  void (*produce)(void *);
  void (*consume)(void *);
  void (*init)(int);
}pool;


task_t* task_alloc(){
  return pmm->alloc(sizeof(task_t));
}

void produce(void *arg){
  while(1){
    P(&pool.last);
    putch('(');
    V(&pool.having);
  }
  
}

void consume(void *arg){
  while(1){
    P(&pool.having);
    putch(')');
    V(&pool.last);
  }
  
}


void pool_init(int size, int con, int pro){
  kmt->sem_init(&pool.last, "pool last", size);
  kmt->sem_init(&pool.having, "pool having", 0);
  for(int i=0;i<pro;i++){
    task_t *task = task_alloc();
    atom_printf("%p", task);
    kmt->create(task,"producer", produce, NULL);
  }
  for(int i=0;i<con;i++){
    task_t *task = task_alloc();
    atom_printf("%p", task);
    kmt->create(task,"consume", consume, NULL);
  }
}




void tty_reader(void *arg) {
  device_t *tty = dev->lookup(arg);

  char cmd[128], resp[128], ps[16];
  snprintf(ps, 16, "(%s) $ ", arg);
  while (1) {
    tty->ops->write(tty, 0, ps, strlen(ps));
    int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
    cmd[nread] = '\0';
    sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
    tty->ops->write(tty, 0, resp, strlen(resp));
  }
}


static void os_init() {
  

  os_irq(0, EVENT_ERROR, error_handler);
  os_irq(50, EVENT_IRQ_IODEV, iodev_handler);
  os_irq(100, EVENT_PAGEFAULT, page_handler);
  os_irq(100, EVENT_SYSCALL, syscall_handler);
  os_irq(100, EVENT_IRQ_TIMER, inter_handler);
  os_irq(100, EVENT_YIELD, yield_handler);
  

  
}

static void os_run() {
  
  atom_printf("os_run() from %d CPU\n", cpu_current());
  iset(true);// 开中断
  while (1);
  

}







//EVENT_SYSCALL
static Context *syscall_handler(Event ev, Context *ctx){
  // TODO

  switch (ctx->rax)
  {
  case SYS_kputc:
    ctx->GPRx =(int)uproc->kputc(current_task, (char)ctx->GPR1);
    break;
  case SYS_fork:
    uproc->fork(current_task); // 这个不能规定返回值
    break;
  case SYS_exit:
    ctx->GPRx =(int)uproc->exit(current_task, (int)ctx->GPRx);
    break;
  case SYS_wait:
    ctx->GPRx =(int)uproc->wait(current_task, (int *)&ctx->GPRx);
    break;
  case SYS_mmap:
    ctx->GPRx =(uint64_t)uproc->mmap(current_task, (void *)ctx->GPRx, (int)ctx->GPR1, (int)ctx->GPR2, (int)ctx->GPR3);
    break;
  case SYS_getpid:
    
    ctx->GPRx =(int)uproc->getpid(current_task);
    break;
  case SYS_sleep:
    ctx->GPRx = (int)uproc->sleep(current_task, (int)ctx->GPRx);
    break;
  case SYS_kill:
    ctx->GPRx = (int)uproc->kill(current_task, (int)ctx->GPRx);
    break;
  case SYS_uptime:
    ctx->GPRx = (int64_t)uproc->uptime(current_task);
    break;
  default:
    break;
  }
  return ctx;
}







// EVENT_PAGEFAULT 将旧的pa减少引用次数，将va映射成旧的pa的拷贝，更改log
static Context *page_handler(Event ev, Context *ctx){
  atom_printf("%p\n", ev.ref);
  
  int index=0;
  void * va = (void *)ROUNDDOWN(ev.ref, current_task->as.pgsize);

  for(;index<current_task->log_len;index++){
    if(current_task->log[index].va == (uintptr_t)va) break;
  }
  
  panic_on(index==current_task->log_len,"page_handler");

  void *pa_old = (void *)current_task->log[index].pa;
  void *pa_new = (void *)pmm->alloc(current_task->as.pgsize);

  memcpy(pa_new, pa_old, current_task->as.pgsize);

  map(&current_task->as, va, NULL, MMAP_NONE);
  map(&current_task->as, va, pa_new, MMAP_READ|MMAP_WRITE);
  current_task->log[index].pa = (uintptr_t)pa_new;
  decrease(pa_old);
  increase(pa_new);

  return ctx;

}




//EVENT_IRQ_IODEV
static Context *iodev_handler(Event ev, Context *ctx){
  
  return ctx;
}


// EVENT_ERROR
static Context *error_handler(Event ev, Context *ctx){
  panic("error_handler");
}


// 时钟中断处理程序
static Context *inter_handler(Event ev, Context *ctx){
  // putch('i');
  
  yield(); // 将执行到这里的状态保存起来，待调用
  
  return ctx;


}

static Context *yield_handler(Event ev, Context *ctx){
  //save
  // putch('y');
  
  // atom_printf("yield_handler\n");
  if(current_task != NULL){ 
    kmt->spin_lock(&current_task->lock);
    if(current_task->status==DEAD) {
      current_task->status=CAN_BE_CLEAR;
      kmt->spin_unlock(&current_task->lock);
      kmt->teardown(current_task);
      current_task = task_list.head;
    }else{
      current_task->context = ctx;
      current_task->status = RUNNABLE;
      
      kmt->spin_unlock(&current_task->lock);
    }  
    // atom_printf("from%s\n", current_task->name);      
  }
  else {
    current_task = task_list.head;
  }
  //schedule

  if(current_task==NULL){// 到这里还是NULL的话，没有task被创建，直接返回ctx
    return ctx;
  }else{ 
    kmt->spin_lock(&task_list.lock); // 锁整个链表
    do{
      current_task=current_task->next;
    }while (current_task->status!=RUNNABLE);
    current_task->status=RUNNING;
    kmt->spin_unlock(&task_list.lock);
    // atom_printf("turn to%s\n", current_task->name);
    return current_task->context;
  } 
  
}



// 中断发生后会到此处
static Context * os_trap(Event ev, Context *context){// 在此处，状态已经被保存在context
  // putch('t');
  Context *next = NULL;
  bool saved_i = ienabled();
  iset(false); // 关中断
  for (handler_node *h=handler_head;h;h=h->next) {
    if (h->event == ev.event) {
      Context *r = h->handler(ev, context); // 用os_irq注册的handler
      panic_on(r && next, "returning multiple contexts");
      if (r) next = r;
    }
  }
  
  panic_on(!next, "returning NULL context");
  // panic_on(sane_context(next), "returning to invalid context");// 检查next（不检查了）

  iset(saved_i);
  return next;
}


// 将handler插入到以seq排序的链表
static void os_irq(int seq, int event, handler_t handler){
  handler_node * add = pmm->alloc(sizeof(handler_node));
  *add = (handler_node){seq,event,handler,NULL};
  if(handler_head == NULL){
    handler_head = add;
  }else{
    if(handler_head->seq > add->seq){
      add->next = handler_head;
      handler_head = add;
    }else{
      handler_node * temp = handler_head;
      while(temp->next != NULL && temp->next->seq < add->seq){
        temp = temp->next;
      }
      add->next = temp->next;
      temp->next = add;
    }
  }
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_irq
};
