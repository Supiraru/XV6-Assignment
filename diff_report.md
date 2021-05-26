# xv6 Code Modification Project 2

## Makefile
```diff
diff --git a/Source-code/Makefile b/Source-code/Makefile
index 0c155f6..45a375a 100644
--- a/Source-code/Makefile
+++ b/Source-code/Makefile
@@ -1,7 +1,7 @@
 # Set flag to correct CS333 project number: 1, 2, ...
 # 0 == original xv6-pdx distribution functionality
-CS333_PROJECT ?= 1
-PRINT_SYSCALLS ?= 1
+CS333_PROJECT ?= 2
+PRINT_SYSCALLS ?= 0
 CS333_CFLAGS ?= -DPDX_XV6
 ifeq ($(CS333_CFLAGS), -DPDX_XV6)
 CS333_UPROGS +=	_halt _uptime
@@ -18,7 +18,7 @@ endif
```

## UID, GID and PPID
```diff
diff --git a/Source-code/proc.c b/Source-code/proc.c
index 228a7e5..9ffdeee 100644
--- a/Source-code/proc.c
+++ b/Source-code/proc.c
@@ -6,6 +6,9 @@
 #include "x86.h"
 #include "proc.h"
 #include "spinlock.h"
+#ifdef CS333_P2
+#include "pdx.h"
+#endif
 
 static char *states[] = {
   [UNUSED]    "unused",
@@ -149,6 +152,8 @@ allocproc(void)
   memset(p->context, 0, sizeof *p->context);
   p->context->eip = (uint)forkret;
   p->start_ticks = ticks;
+  p->cpu_ticks_in = 0;
+  p->cpu_ticks_total = 0;
 
   return p;
 }
@@ -177,6 +182,11 @@ userinit(void)
   p->tf->esp = PGSIZE;
   p->tf->eip = 0;  // beginning of initcode.S
 
+  #ifdef CS333_P2
+  p->uid = DEFAULT_UID;
+  p->gid = DEFAULT_GID;
+  #endif
+
   safestrcpy(p->name, "initcode", sizeof(p->name));
   p->cwd = namei("/");
 
@@ -248,6 +258,8 @@ fork(void)
   safestrcpy(np->name, curproc->name, sizeof(curproc->name));
 
   pid = np->pid;
+  np->uid = curproc->uid;
+  np->gid = curproc->gid;
 
   acquire(&ptable.lock);
   np->state = RUNNABLE;


diff --git a/Source-code/proc.h b/Source-code/proc.h
index c7ee129..3811f7d 100644
--- a/Source-code/proc.h
+++ b/Source-code/proc.h
@@ -41,6 +41,8 @@ struct proc {
   char *kstack;                // Bottom of kernel stack for this process
   enum procstate state;        // Process state
   uint pid;                    // Process ID
+  uint uid;
+  uint gid;
   struct proc *parent;         // Parent process. NULL indicates no parent
   struct trapframe *tf;        // Trap frame for current syscall
   struct context *context;     // swtch() here to run process

diff --git a/Source-code/runoff1 b/Source-code/runoff1
old mode 100755
new mode 100644
diff --git a/Source-code/show1 b/Source-code/show1
old mode 100755
new mode 100644
diff --git a/Source-code/spinp b/Source-code/spinp
old mode 100755
new mode 100644
diff --git a/Source-code/syscall.c b/Source-code/syscall.c
index e3543f1..5ee0f7d 100644
--- a/Source-code/syscall.c
+++ b/Source-code/syscall.c
@@ -109,6 +109,13 @@ extern int sys_halt(void);
 #ifdef CS333_P1
   extern int sys_date(void);
 #endif
+#ifdef CS333_P2
+  extern int sys_getuid(void);
+  extern int sys_getgid(void);
+  extern int sys_getppid(void);
+  extern int sys_setuid(void);
+  extern int sys_setgid(void);
+#endif
 
 static int (*syscalls[])(void) = {
 [SYS_fork]    sys_fork,
@@ -138,6 +145,13 @@ static int (*syscalls[])(void) = {
 #ifdef CS333_P1
   [SYS_date]    sys_date,
 #endif
+#ifdef CS333_P2
+  [SYS_getuid]    sys_getuid,
+  [SYS_getgid]    sys_getgid,
+  [SYS_getppid]    sys_getppid,
+  [SYS_setuid]    sys_setuid,
+  [SYS_setgid]    sys_setgid,
+#endif
 };
 
 #ifdef PRINT_SYSCALLS
@@ -166,6 +180,16 @@ static char *syscallnames[] = {
 #ifdef PDX_XV6
   [SYS_halt]    "halt",
 #endif // PDX_XV6
+#ifdef CS333_P1
+  [SYS_date]    "date",
+#endif
+#ifdef CS333_P2
+  [SYS_getuid]    "getuid",
+  [SYS_getgid]    "getgid",
+  [SYS_getppid]    "getppid",
+  [SYS_setuid]    "setuid",
+  [SYS_setgid]    "setgid",
+#endif
 };
 #endif // PRINT_SYSCALLS
 
diff --git a/Source-code/syscall.h b/Source-code/syscall.h
index 14f3e17..9f3e692 100644
--- a/Source-code/syscall.h
+++ b/Source-code/syscall.h
@@ -23,3 +23,8 @@
 #define SYS_halt    SYS_close+1
 // student system calls begin here. Follow the existing pattern.
 #define SYS_date    SYS_halt+1
+#define SYS_getuid    SYS_date+1
+#define SYS_getgid    SYS_getuid+1
+#define SYS_getppid    SYS_getgid+1
+#define SYS_setuid    SYS_getppid+1
+#define SYS_setgid    SYS_setuid+1
diff --git a/Source-code/sysproc.c b/Source-code/sysproc.c
index a7fcfed..0b33317 100644
--- a/Source-code/sysproc.c
+++ b/Source-code/sysproc.c
@@ -97,7 +97,7 @@ sys_halt(void)
 }
 #endif // PDX_XV6
 
-
+#ifdef CS333_P1
 int
 sys_date(void)
 {
@@ -110,3 +110,52 @@ sys_date(void)
   cmostime(d);
   return 0;
 }
+#endif
+
+#ifdef CS333_P2
+int
+sys_getuid(void)
+{
+  return myproc()->uid;
+}
+
+int
+sys_getgid(void)
+{
+  return myproc()->gid;
+}
+
+int
+sys_getppid(void)
+{
+  if(myproc()->pid != 1){
+    return myproc()->parent->pid; 
+  }
+  return myproc()->pid;
+}
+
+int
+sys_setuid(void)
+{
+  int var;
+
+  if (argint(0, &var) < 0 || var > 32767 || var < 0){
+    return -1;
+  }
+  myproc()->uid = (uint)var;
+  return 0;
+}
+
+int
+sys_setgid(void)
+{
+  int var;
+
+  if (argint(0, &var) < 0 || var > 32767 || var < 0){
+    return -1;
+  }
+  myproc()->gid = (uint)var;
+  return 0;
+}
+
+#endif
\ No newline at end of file
diff --git a/Source-code/user.h b/Source-code/user.h
index 631d07e..f994fb1 100644
--- a/Source-code/user.h
+++ b/Source-code/user.h
@@ -29,6 +29,14 @@ int halt(void);
     int date(struct rtcdate*);
 #endif
 
+#ifdef CS333_P2
+    uint getuid(void);
+    uint getgid(void);
+    uint getppid(void);
+    int setuid(uint);
+    int setgid(uint);
+#endif
+
 
 // ulib.c
 int stat(char*, struct stat*);
diff --git a/Source-code/usys.S b/Source-code/usys.S
index 84bd80b..090de9a 100644
--- a/Source-code/usys.S
+++ b/Source-code/usys.S
@@ -31,3 +31,8 @@ SYSCALL(sleep)
 SYSCALL(uptime)
 SYSCALL(halt)
 SYSCALL(date)
+SYSCALL(getuid)
+SYSCALL(getgid)
+SYSCALL(getppid)
+SYSCALL(setuid)
+SYSCALL(setgid)

```


## Process Execution Time
```diff
diff --git a/Source-code/proc.c b/Source-code/proc.c
index 228a7e5..9ffdeee 100644
--- a/Source-code/proc.c
+++ b/Source-code/proc.c
@@ -149,6 +152,8 @@ allocproc(void)
   memset(p->context, 0, sizeof *p->context);
   p->context->eip = (uint)forkret;
   p->start_ticks = ticks;
+  p->cpu_ticks_in = 0;
+  p->cpu_ticks_total = 0;
 
   return p;
@@ -390,6 +402,7 @@ scheduler(void)
       c->proc = p;
       switchuvm(p);
       p->state = RUNNING;
+      p->cpu_ticks_in = ticks;
       swtch(&(c->scheduler), p->context);
       switchkvm();
 
@@ -430,7 +443,10 @@ sched(void)
   if(readeflags()&FL_IF)
     panic("sched interruptible");
   intena = mycpu()->intena;
+
+  p->cpu_ticks_total += (ticks - p->cpu_ticks_in);
   swtch(&p->context, mycpu()->scheduler);
+  
   mycpu()->intena = intena;
 }

diff --git a/Source-code/proc.h b/Source-code/proc.h
index c7ee129..3811f7d 100644
--- a/Source-code/proc.h
+++ b/Source-code/proc.h


 @@ -50,6 +52,8 @@ struct proc {
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
   uint start_ticks;
+  uint cpu_ticks_total;
+  uint cpu_ticks_in;
 };
 
 // Process memory is laid out contiguously, low addresses first:


```

## PS command
``` diff
diff --git a/Source-code/defs.h b/Source-code/defs.h
index f85557d..9f099de 100644
--- a/Source-code/defs.h
+++ b/Source-code/defs.h
@@ -9,6 +9,8 @@ struct spinlock;
 struct sleeplock;
 struct stat;
 struct superblock;
+struct uproc;
+
 
 // bio.c
 void            binit(void);
@@ -124,6 +126,9 @@ void            userinit(void);
 int             wait(void);
 void            wakeup(void*);
 void            yield(void);
+#ifdef CS333_P2
+int             copy(int, struct uproc*);
+#endif // CS333_p2
 #ifdef CS333_P3
 void            printFreeList(void);
 void            printList(int);

diff --git a/Source-code/proc.c b/Source-code/proc.c
index 9ffdeee..36b04af 100644
--- a/Source-code/proc.c
+++ b/Source-code/proc.c
@@ -5,6 +5,7 @@
 #include "mmu.h"
 #include "x86.h"
 #include "proc.h"
+#include "uproc.h"
 #include "spinlock.h"
 #ifdef CS333_P2
 #include "pdx.h"
@@ -628,7 +629,49 @@ procdumpP1(struct proc *p, char *state_string)
   }
   return;
 }
+
+#ifdef CS333_P2
+// Helper function to access ptable for sys_getprocs
+int
+copy(int max, struct uproc* up)
+{
+ acquire(&ptable.lock);
+ int counter = 0;
+ struct proc* testProc;
+
+ for(testProc = ptable.proc; testProc < &ptable.proc[NPROC]; testProc++){
+   if (counter == max)
+     break;
+   if (testProc->state == EMBRYO || testProc->state == UNUSED){
+     continue;
+   }else if(testProc->state == SLEEPING || testProc->state == RUNNABLE || testProc->state == RUNNING || testProc->state == ZOMBIE) {
+     up[counter].pid = testProc->pid;
+     up[counter].uid = testProc->uid;
+     up[counter].gid = testProc->gid;
+
+     // Handle init PPID
+     if (testProc->pid != 1){
+       up[counter].ppid = testProc->parent->pid;
+     }else{
+       up[counter].ppid = testProc->pid;
+     }
+
+     up[counter].CPU_total_ticks = testProc->cpu_ticks_total;
+     up[counter].elapsed_ticks = ticks - testProc->start_ticks;
+     safestrcpy(up[counter].state, states[testProc->state], sizeof(up[counter].state));
+     up[counter].size = testProc->sz;
+     safestrcpy(up[counter].name, (char*)testProc->name, sizeof(testProc->name));
+ 
+     counter++;
+   }
+ }
+
+ release(&ptable.lock);
+ return counter;
+}
+#endif // CS333_P2
+
 
diff --git a/Source-code/ps.c b/Source-code/ps.c
new file mode 100644
index 0000000..d9a1916
--- /dev/null
+++ b/Source-code/ps.c
@@ -0,0 +1,68 @@
+#ifdef CS333_P2
+
+#define MAX_ENTRIES 16
+
+#include "types.h"
+#include "user.h"
+#include "uproc.h"
+
+int
+main(int argc, char *argv[])
+{
+  struct uproc* data;
+
+  data = malloc(sizeof(struct uproc) * MAX_ENTRIES);
+
+  if (getprocs(MAX_ENTRIES, data) < 0) {
+    printf(2,"Error: getprocs call failed. %s at line %d\n",
+        __FILE__, __LINE__);
+    exit();
+  }
+
+  printf(1, "PID\tName\t\tUID\tGID\tPPID\tElapsed\tCPU\tState\tSize\n");
+
+  for (int i = 0; i < MAX_ENTRIES; i++){
+    if (data[i].pid == NULL)
+      break;
+    printf(1, "%d\t%s\t\t%d\t%d\t%d\t", 
+      data[i].pid,
+      data[i].name,
+      data[i].uid,
+      data[i].gid,
+      data[i].ppid
+    );
+
+    // Special case for floating point
+    // Elapsed Ticks
+    if (data[i].elapsed_ticks < 10){
+      printf(1, "0.00%d\t", data[i].elapsed_ticks);
+    } else if (data[i].elapsed_ticks < 100){
+      printf(1, "0.0%d\t", data[i].elapsed_ticks);
+    } else if (data[i].elapsed_ticks < 1000){
+      printf(1, "0.%d\t", data[i].elapsed_ticks);
+    }else{
+      printf(1, "%d.%d\t", data[i].elapsed_ticks/1000, data[i].elapsed_ticks%1000);
+    }
+
+    // CPU Total Ticks
+    if (data[i].CPU_total_ticks < 10){
+      printf(1, "0.00%d", data[i].CPU_total_ticks);
+    } else if (data[i].CPU_total_ticks < 100){
+      printf(1, "0.0%d", data[i].CPU_total_ticks);
+    } else if (data[i].CPU_total_ticks < 1000){
+      printf(1, "0.%d", data[i].CPU_total_ticks);
+    }else{
+      printf(1, "%d.%d", data[i].CPU_total_ticks/1000, data[i].CPU_total_ticks%1000);
+    }
+
+    printf(1, "\t%s\t%d\n",
+      data[i].state,
+      data[i].size
+    );
+  }
+
+  printf(1, "\n");
+  exit();
+}
+
+#endif // CS333_P2
diff --git a/Source-code/syscall.c b/Source-code/syscall.c
index 5ee0f7d..754e63e 100644
--- a/Source-code/syscall.c
+++ b/Source-code/syscall.c
@@ -115,6 +115,7 @@ extern int sys_halt(void);
   extern int sys_getppid(void);
   extern int sys_setuid(void);
   extern int sys_setgid(void);
+  extern int sys_getprocs(void);
 #endif
 
 static int (*syscalls[])(void) = {
@@ -151,6 +152,8 @@ static int (*syscalls[])(void) = {
   [SYS_getppid]    sys_getppid,
   [SYS_setuid]    sys_setuid,
   [SYS_setgid]    sys_setgid,
+  [SYS_getprocs]  sys_getprocs
+
 #endif
 };
 
@@ -184,11 +187,13 @@ static char *syscallnames[] = {
   [SYS_date]    "date",
 #endif
 #ifdef CS333_P2
-  [SYS_getuid]    "getuid",
-  [SYS_getgid]    "getgid",
+  [SYS_getuid]     "getuid",
+  [SYS_getgid]     "getgid",
   [SYS_getppid]    "getppid",
-  [SYS_setuid]    "setuid",
-  [SYS_setgid]    "setgid",
+  [SYS_setuid]     "setuid",
+  [SYS_setgid]     "setgid",
+  [SYS_getprocs]   "getprocs",
+
 #endif
 };
 #endif // PRINT_SYSCALLS
diff --git a/Source-code/syscall.h b/Source-code/syscall.h
index 9f3e692..a587628 100644
--- a/Source-code/syscall.h
+++ b/Source-code/syscall.h
@@ -28,3 +28,4 @@
 #define SYS_getppid    SYS_getgid+1
 #define SYS_setuid    SYS_getppid+1
 #define SYS_setgid    SYS_setuid+1
+#define SYS_getprocs  SYS_setgid+1
diff --git a/Source-code/sysproc.c b/Source-code/sysproc.c
index 0b33317..7316600 100644
--- a/Source-code/sysproc.c
+++ b/Source-code/sysproc.c
@@ -9,6 +9,9 @@
 #ifdef PDX_XV6
 #include "pdx-kernel.h"
 #endif // PDX_XV6
+#ifdef CS333_P2
+#include "uproc.h"
+#endif
 
 int
 sys_fork(void)
@@ -158,4 +161,15 @@ sys_setgid(void)
   return 0;
 }
 
+int
+sys_getprocs(void)
+{
+  int max;
+  struct uproc* up;
+
+  if (argint(0, &max) < 0 || argptr(1, (void*)&up, sizeof(struct uproc) * max) < 0)
+    return -1;
+
+  return copy(max, up);
+}
 #endif
\ No newline at end of file

```

## Time
``` diff
+++ b/Source-code/time.c
@@ -0,0 +1,44 @@
+// uptime. How long has xv6 been up
+#include "types.h"
+#include "user.h"
+
+int
+main(int argc, char *argv[])
+{
+  int start_time = uptime();
+
+  if (argc == 1){
+      int rem = 0;
+      int current_time = uptime() - start_time;
+      if(current_time > 1000){
+        rem = current_time % 1000;
+        current_time /= 1000;
+    }
+      if(rem != 0){
+        printf(1, "(null) ran in %d.%d seconds\n", current_time, rem);
+    } else{
+        printf(1, "(null) ran in 0.%d seconds\n", current_time);
+    }
+        //if doesnt have any arg
+    } else {
+        if (fork() == 0){
+            exec(argv[1], &argv[1]);
+        }else{
+            wait();
+             int rem = 0;
+             int current_time = uptime() - start_time;
+            
+            if(current_time > 1000){
+            rem = current_time % 1000;
+            current_time /= 1000;
+            }
+            if(rem == 0){
+                printf(1, "%s ran in 0.%d seconds\n", argv[1], current_time);
+            } else{
+                printf(1, "%s ran in %d.%d seconds\n", argv[1], current_time, rem);
+            }
+        }
+    }
+
+  exit();
+}
diff --git a/Source-code/runoff.list b/Source-code/runoff.list
index f291b68..d0e886d 100644
--- a/Source-code/runoff.list
+++ b/Source-code/runoff.list
@@ -93,3 +93,4 @@ p3-test.c
 p4-test.c
 testsetprio.c
 ps.c
+time.c

```

## Modifying the Console
``` diff
@@ -557,7 +573,42 @@ kill(int pid)
 void
 procdumpP2P3P4(struct proc *p, char *state_string)
 {
-  cprintf("TODO for Project 2, delete this line and implement procdumpP2P3P4() in proc.c to print a row\n");
+  cprintf("%d\t%s\t\t%d\t%d\t", p->pid, p->name, p->uid, p->gid);
+  
+  if(p->pid == 1){
+    cprintf("%d\t", p->pid);
+  }
+  else{
+    cprintf("%d\t", p->parent->pid);
+  }
+
+  
+  int rem = 0;
+  int current_ticks = ticks - (p->start_ticks);
+  if(current_ticks > 1000){
+    rem = current_ticks % 1000;
+    current_ticks = current_ticks / 1000;
+  }
+
+  if(rem == 0){
+    cprintf("0.%d\t", current_ticks);
+  }else{
+    cprintf("%d.%d\t", current_ticks, rem);
+  }
+
+  int cpu_rem = 0;
+  int current_cpu_ticks = p->cpu_ticks_total;
+  if(current_cpu_ticks > 1000){
+    cpu_rem = current_cpu_ticks % 1000;
+    current_cpu_ticks = current_cpu_ticks / 1000;
+  }
+
+  if(cpu_rem == 0){
+    cprintf("0.%d\t%s\t%d\t", current_cpu_ticks, states[p->state], p->sz);
+  }else{
+    cprintf("%d.%d\t%s\t%d\t", current_cpu_ticks, cpu_rem, states[p->state], p->sz);
+  }
+  
   return;
 }
 #elif defined(CS333_P1)

```