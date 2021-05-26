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

```

## PS command
``` diff

```
