#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_prioritize(void)
{
  int priority;
  argint(0, &priority);

  prioritize(priority);
  return 0;
}

uint64
sys_priofork(void)
{
  int priority;
  argint(0, &priority);

  return priofork(priority);
}

uint64
sys_read_sensor(void)
{
  uint current_ticks;
  static int read_count = 0;

  read_count++;

  acquire(&tickslock);
  current_ticks = ticks; // Global ticks variable, 100Hz (ticks / sec)
  release(&tickslock);

  // If in a small window (< 5) of the cycle, simulate a massive increase in speed:
  if (read_count % 10 == 0) {
      return 150; // Return a crazy high spinning value!!! wooaohhh
  }

  return 10 + (current_ticks % 20); // Normally, return a slightly fluctauating spinning speed (10 to 29)
}

#define LOG_SIZE 256
#define LOG_EVENT_NORMAL 0
#define LOG_EVENT_SPIKE 1
#define LOG_EVENT_SHUTDOWN 2

struct log_entry {
  uint timestamp;
  int rpm;
  int event;
};

struct turbine_log {
  int log_head;
  int log_count;
  struct log_entry log_buf[LOG_SIZE];
  
};

static struct spinlock log_lock;
static struct turbine_log log;

uint64 sys_write_log(void) {
  int rpm;
  int event;
  uint current_ticks;

  // get the rpm and event from the syscall
  argint(0, &rpm);
  argint(1, &event);

  // get the tick count
  acquire(&tickslock);
  current_ticks = ticks;
  release(&tickslock);

  // add the log entry to the log buffer
  acquire(&log_lock);
  log.log_buf[log.log_head].timestamp = current_ticks;
  log.log_buf[log.log_head].rpm = rpm;
  log.log_buf[log.log_head].event = event;
  // treat the log buffer as a circular buffer
  log.log_head = (log.log_head + 1) % LOG_SIZE;
  // if the circular is not full, we are safe to increment the log count
  if (log.log_count < LOG_SIZE) {
    log.log_count++;
  }
  release(&log_lock);

  return 0;
}

uint64 sys_get_log(void) {
  uint64 uaddr;
  argaddr(0, &uaddr);

  // copy the log struct to userspace
  if (copyout(myproc()->pagetable, uaddr, (char*)&log, sizeof(log)) < 0) {
    return -1;
  }
  return 0;
}
