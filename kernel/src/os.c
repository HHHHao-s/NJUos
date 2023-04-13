// #include <common.h>
#include <os.h>

static void os_run();
static Context * os_trap(Event ev, Context *context);
static void os_irq(int seq, int event, handler_t handler);
static Context *inter_handler(Event ev, Context *ctx);
static Context *yield_handler(Event ev, Context *ctx);
spinlock_t print_lock;
#define atom_printf(...)  \
  kmt->spin_lock(&print_lock);\
  printf(__VA_ARGS__);\
  kmt->spin_unlock(&print_lock)

// extern struct task;

static void os_init() {
  pmm->init();
  os_irq(100, EVENT_IRQ_TIMER, inter_handler);
  os_irq(100, EVENT_YIELD, yield_handler);
  kmt->init();

  
  kmt->spin_init(&print_lock,"print_lock");



  // for(int i=0;i<MAX_TASKS;i++){
  //   taskarr->tasks[i] = NULL;
  // }
  // taskarr->len = 0;
  
  // for(int64_t i=0;i<9;i++){ // 创建10个task
  //   task_t *task = pmm->alloc(sizeof(task_t));
  //   task->entry = os_run;
  //   task->status = RUNNABLE;
  //   Area stack    = (Area) { &task->stack, task + 1 };
  //   task->context = kcontext(stack, task->entry, (void *)i);
  //   task->next    = NULL;
  //   taskarr->tasks[taskarr->len++] = task;
  // }
  // for(int i=0;i<9;i++){
  //   task_t *task =  taskarr->tasks[i];
    
  //   task->next =  taskarr->tasks[(i+1)% taskarr->len];
  // }
  // yield();
  
  // while (1);
}

static void os_run() {
  int cpu = cpu_current();
  atom_printf("os_run() from %d CPU", cpu);
  iset(true);// 开中断
  while (1);
  

}

// static void test(void *num) {

//   while (1){
//     putch((int64_t)num);
//     for (int volatile i = 0; i < 100000; i++) ; // sleep
//   }

// }

// 时钟中断处理程序
static Context *inter_handler(Event ev, Context *ctx){
  putch('i');
  yield(); // 将执行到这里的状态保存起来，待调用
  return ctx;


}

static Context *yield_handler(Event ev, Context *ctx){
  //save
  putch('y');
  if(current_task == NULL){
    // P 一个task来做
  }
  else {
    current_task->context = ctx;
    current_task->status = RUNNABLE;
  }
  //schedule
  do{
    current_task=current_task->next;
  }while (current_task->status!=RUNNABLE);
  current_task->status=RUNNING;
  return current_task->context;
  
}



// 中断发生后会到此处
static Context * os_trap(Event ev, Context *context){// 在此处，状态已经被保存在context
  putch('t');
  Context *next = NULL;
  for (handler_node *h=handler_head;h;h=h->next) {
    if (h->event == ev.event) {
      Context *r = h->handler(ev, context); // 用os_irq注册的handler
      panic_on(r && next, "returning multiple contexts");
      if (r) next = r;
    }
  }
  putch('d');
  panic_on(!next, "returning NULL context");
  // panic_on(sane_context(next), "returning to invalid context");// 检查next（不检查了）
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
