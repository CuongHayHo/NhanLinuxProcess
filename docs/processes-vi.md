# Tiến trình (Processes) — Bản dịch tiếng Việt

> Bản dịch tiếng Việt của bài giảng **"Processes"** từ Linux Kernel Labs.
> Nguồn gốc: <https://linux-kernel-labs.github.io/refs/heads/master/lectures/processes.html>
>
> Đây là phần lý thuyết nền tảng trực tiếp liên quan đến yêu cầu *"Lập trình quản lý tiến trình"* và *"Lập trình xây dựng một mô-đun nhân và tích hợp vào hệ thống"* của đồ án. Các đoạn mã nguồn (C / hợp ngữ / gdb) được giữ nguyên bản gốc; phần diễn giải được dịch sang tiếng Việt.

---

## Mục tiêu bài giảng (Lecture objectives)

- Tiến trình (process) và luồng (thread)
- Chuyển ngữ cảnh (context switching)
- Chặn (blocking) và đánh thức (waking up) tác vụ
- Ngữ cảnh tiến trình (process context)

---

## Tiến trình và luồng (Processes and threads)

Tiến trình (process) là một khái niệm trừu tượng của hệ điều hành, dùng để **nhóm lại nhiều tài nguyên** với nhau:

- Một không gian địa chỉ (address space)
- Một hoặc nhiều luồng (threads)
- Các tập tin đang mở (opened files)
- Các socket
- Các semaphore
- Các vùng bộ nhớ chia sẻ (shared memory regions)
- Các bộ định thời (timers)
- Các bộ xử lý tín hiệu (signal handlers)
- Nhiều tài nguyên và thông tin trạng thái khác

Tất cả thông tin này được gom lại trong **Process Control Block (PCB)**. Trong Linux, đây chính là cấu trúc `struct task_struct`.

### Tổng quan về tài nguyên của tiến trình (Overview of process resources)

Có thể lấy tóm tắt về các tài nguyên mà một tiến trình đang sở hữu từ thư mục `/proc/<pid>`, trong đó `<pid>` là mã số (process id) của tiến trình mà ta muốn xem xét.

```
                +-------------------------------------------------------------------+
                | dr-x------    2 tavi tavi 0  2021 03 14 12:34 .                   |
                | dr-xr-xr-x    6 tavi tavi 0  2021 03 14 12:34 ..                  |
                | lrwx------    1 tavi tavi 64 2021 03 14 12:34 0 -> /dev/pts/4     |
           +--->| lrwx------    1 tavi tavi 64 2021 03 14 12:34 1 -> /dev/pts/4     |
           |    | lrwx------    1 tavi tavi 64 2021 03 14 12:34 2 -> /dev/pts/4     |
           |    | lr-x------    1 tavi tavi 64 2021 03 14 12:34 3 -> /proc/18312/fd |
           |    +-------------------------------------------------------------------+
           |                 +----------------------------------------------------------------+
           |                 | 08048000-0804c000 r-xp 00000000 08:02 16875609 /bin/cat        |
$ ls -1 /proc/self/          | 0804c000-0804d000 rw-p 00003000 08:02 16875609 /bin/cat        |
cmdline    |                 | 0804d000-0806e000 rw-p 0804d000 00:00 0 [heap]                 |
cwd        |                 | ...                                                            |
environ    |    +----------->| b7f46000-b7f49000 rw-p b7f46000 00:00 0                        |
exe        |    |            | b7f59000-b7f5b000 rw-p b7f59000 00:00 0                        |
fd --------+    |            | b7f5b000-b7f77000 r-xp 00000000 08:02 11601524 /lib/ld-2.7.so  |
fdinfo          |            | b7f77000-b7f79000 rw-p 0001b000 08:02 11601524 /lib/ld-2.7.so  |
maps -----------+            | bfa05000-bfa1a000 rw-p bffeb000 00:00 0 [stack]                |
mem                          | ffffe000-fffff000 r-xp 00000000 00:00 0 [vdso]                 |
root                         +----------------------------------------------------------------+
stat                 +----------------------------+
statm                |  Name: cat                 |
status ------+       |  State: R (running)        |
task         |       |  Tgid: 18205               |
wchan        +------>|  Pid: 18205                |
                     |  PPid: 18133               |
                     |  Uid: 1000 1000 1000 1000  |
                     |  Gid: 1000 1000 1000 1000  |
                     +----------------------------+
```

### struct task_struct

Hãy xem xét kỹ cấu trúc `struct task_struct`. Để làm việc đó ta có thể đọc trực tiếp mã nguồn, nhưng ở đây ta sẽ dùng một công cụ tên là `pahole` (thuộc gói cài đặt `dwarves`) để có thêm hiểu biết về cấu trúc này:

```
$ pahole -C task_struct vmlinux

struct task_struct {
    struct thread_info thread_info;                  /*     0     8 */
    volatile long int          state;                /*     8     4 */
    void *                     stack;                /*    12     4 */

    ...

    /* --- cacheline 45 boundary (2880 bytes) --- */
    struct thread_struct thread __attribute__((__aligned__(64))); /*  2880  4288 */

    /* size: 7168, cachelines: 112, members: 155 */
    /* sum members: 7148, holes: 2, sum holes: 12 */
    /* sum bitfield members: 7 bits, bit holes: 2, sum bit holes: 57 bits */
    /* paddings: 1, sum paddings: 2 */
    /* forced alignments: 6, forced holes: 2, sum forced holes: 12 */
} __attribute__((__aligned__(64)));
```

Như bạn thấy, đây là một cấu trúc dữ liệu khá lớn: kích thước gần **8KB** và có **155 trường (fields)**.

### Khảo sát task_struct (Inspecting task_struct)

Đoạn screencast (trong bài gốc) minh họa cách khảo sát khối điều khiển tiến trình (`struct task_struct`) bằng cách kết nối trình gỡ lỗi (debugger) với máy ảo đang chạy. Ta sẽ dùng một lệnh gdb hỗ trợ là `lx-ps` để liệt kê các tiến trình cùng địa chỉ `task_struct` của mỗi tiến trình.

### Câu hỏi: Khảo sát một tác vụ để xác định các tập tin đang mở

Dùng debugger để khảo sát tiến trình có tên `syslogd`.

- Cần dùng lệnh nào để liệt kê các bộ mô tả tập tin (file descriptors) đang mở?
- Có bao nhiêu bộ mô tả tập tin đang mở?
- Cần dùng lệnh nào để xác định tên tập tin ứng với file descriptor số 3 đang mở?
- Tên tập tin của file descriptor số 3 là gì?

### Luồng (Threads)

Luồng (thread) là đơn vị cơ bản mà bộ định thời tiến trình (process scheduler) của nhân sử dụng để cho phép các ứng dụng chạy trên CPU. Một luồng có các đặc điểm sau:

- Mỗi luồng có ngăn xếp (stack) riêng, và cùng với các giá trị thanh ghi (register) sẽ xác định trạng thái thực thi của luồng.
- Một luồng chạy trong ngữ cảnh của một tiến trình, và tất cả các luồng trong cùng một tiến trình sẽ chia sẻ tài nguyên với nhau.
- Nhân lập lịch theo **luồng** chứ không phải theo tiến trình, và các luồng ở mức người dùng (ví dụ: fibers, coroutines, ...) không nhìn thấy được ở mức nhân.

Cách hiện thực luồng điển hình là: luồng được hiện thực như một cấu trúc dữ liệu riêng biệt, sau đó được liên kết tới cấu trúc dữ liệu của tiến trình. Ví dụ, nhân Windows dùng kiểu hiện thực như vậy.

Linux dùng một cách hiện thực khác cho luồng. Đơn vị cơ bản được gọi là **task** (do đó mới có tên `struct task_struct`), và nó được dùng cho **cả luồng lẫn tiến trình**. Thay vì nhúng (embed) trực tiếp các tài nguyên vào cấu trúc task, cấu trúc này chứa các **con trỏ (pointers)** trỏ tới những tài nguyên đó.

Do đó, nếu hai luồng thuộc cùng một tiến trình, chúng sẽ trỏ tới cùng một thể hiện (instance) của cấu trúc tài nguyên. Nếu hai luồng thuộc các tiến trình khác nhau, chúng sẽ trỏ tới các thể hiện cấu trúc tài nguyên khác nhau.

### Lời gọi hệ thống clone (The clone system call)

Trong Linux, một luồng hoặc tiến trình mới được tạo ra bằng lời gọi hệ thống `clone()`. Cả lời gọi hệ thống `fork()` lẫn hàm `pthread_create()` đều sử dụng hiện thực của `clone()`.

Lời gọi này cho phép bên gọi quyết định những tài nguyên nào sẽ được chia sẻ với tiến trình cha (parent) và những tài nguyên nào sẽ được sao chép hoặc cô lập:

- `CLONE_FILES` — chia sẻ bảng bộ mô tả tập tin (file descriptor table) với tiến trình cha
- `CLONE_VM` — chia sẻ không gian địa chỉ (address space) với tiến trình cha
- `CLONE_FS` — chia sẻ thông tin hệ thống tập tin (thư mục gốc, thư mục hiện tại) với tiến trình cha
- `CLONE_NEWNS` — **không** chia sẻ mount namespace với tiến trình cha
- `CLONE_NEWIPC` — **không** chia sẻ IPC namespace (các đối tượng System V IPC, hàng đợi thông điệp POSIX) với tiến trình cha
- `CLONE_NEWNET` — **không** chia sẻ networking namespace (giao diện mạng, bảng định tuyến) với tiến trình cha

Ví dụ, nếu bên gọi dùng `CLONE_FILES | CLONE_VM | CLONE_FS` thì thực chất một **luồng mới** được tạo ra. Nếu không dùng các cờ này thì một **tiến trình mới** được tạo ra.

### Namespace và "container"

"Container" là một dạng máy ảo nhẹ (lightweight virtual machine) cùng chia sẻ một thể hiện nhân (kernel instance), khác với ảo hóa thông thường — nơi một hypervisor chạy nhiều máy ảo (VM), mỗi máy có thể hiện nhân riêng của nó.

Ví dụ về công nghệ container là **LXC** — cho phép chạy "VM" nhẹ — và **docker** — một loại container chuyên dụng để chạy một ứng dụng đơn lẻ.

Container được xây dựng dựa trên một vài tính năng của nhân, trong đó có **namespace**. Namespace cho phép cô lập các tài nguyên khác nhau mà nếu không thì sẽ nhìn thấy được trên phạm vi toàn cục. Ví dụ, nếu không có container, mọi tiến trình đều nhìn thấy trong `/proc`. Với container, các tiến trình trong một container sẽ không nhìn thấy được (trong `/proc`, và cũng không thể bị "kill") bởi các container khác.

Để đạt được sự phân vùng này, cấu trúc `struct nsproxy` được dùng để nhóm các loại tài nguyên mà ta muốn phân vùng. Hiện tại nó hỗ trợ các namespace: IPC, networking, cgroup, mount, PID, và time. Ví dụ, thay vì có một danh sách toàn cục cho các giao diện mạng, danh sách này trở thành một phần của `struct net`. Hệ thống khởi tạo với một namespace mặc định (`init_net`) và mặc định mọi tiến trình sẽ chia sẻ namespace này. Khi một namespace mới được tạo, một net namespace mới được tạo ra và các tiến trình mới có thể trỏ tới namespace mới đó thay vì namespace mặc định.

### Truy cập tiến trình hiện tại (Accessing the current process)

Truy cập tiến trình hiện tại là một thao tác rất thường xuyên:

- Mở một tập tin cần truy cập trường `files` của `struct task_struct`
- Ánh xạ (map) một tập tin mới cần truy cập trường `mm` của `struct task_struct`
- Hơn 90% các lời gọi hệ thống cần truy cập cấu trúc tiến trình hiện tại, vì vậy thao tác này cần phải nhanh
- Macro `current` được cung cấp để truy cập `struct task_struct` của tiến trình hiện tại

Để hỗ trợ truy cập nhanh trong các cấu hình đa bộ xử lý (multi-processor), một biến **per-CPU** được dùng để lưu trữ và truy xuất con trỏ tới `struct task_struct` hiện tại.

Trước đây, chuỗi lệnh sau được dùng làm hiện thực cho macro `current`:

```c
/* how to get the current stack pointer from C */
register unsigned long current_stack_pointer asm("esp") __attribute_used__;

/* how to get the thread information struct from C */
static inline struct thread_info *current_thread_info(void)
{
   return (struct thread_info *)(current_stack_pointer & ~(THREAD_SIZE – 1));
}

#define current current_thread_info()->task
```

### Câu hỏi: hiện thực trước đây cho `current` (x86)

- Kích thước của `struct thread_info` là bao nhiêu?
- Trong các kích thước sau, đâu là kích thước hợp lệ tiềm năng cho `struct thread_info`: 4095, 4096, 4097?

---

## Chuyển ngữ cảnh (Context switching)

Sơ đồ (trong bài gốc) cho thấy tổng quan về quy trình chuyển ngữ cảnh của nhân Linux.

Lưu ý rằng trước khi một lần chuyển ngữ cảnh có thể xảy ra, ta phải thực hiện một bước **chuyển sang nhân (kernel transition)**, hoặc bằng một lời gọi hệ thống, hoặc bằng một ngắt (interrupt). Tại thời điểm đó, các thanh ghi của không gian người dùng (user space registers) được lưu lên ngăn xếp nhân (kernel stack). Đến một lúc nào đó hàm `schedule()` sẽ được gọi và có thể quyết định rằng cần phải chuyển ngữ cảnh từ T0 sang T1 (ví dụ: vì luồng hiện tại đang bị chặn để chờ một thao tác I/O hoàn tất, hoặc vì lượng thời gian (time slice) được cấp đã hết).

Tại thời điểm đó, hàm `context_switch()` sẽ thực hiện các thao tác đặc thù theo kiến trúc (architecture specific) và sẽ chuyển đổi không gian địa chỉ nếu cần:

```c
static __always_inline struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
         struct task_struct *next, struct rq_flags *rf)
{
    prepare_task_switch(rq, prev, next);

    /*
     * For paravirt, this is coupled with an exit in switch_to to
     * combine the page table reload and the switch backend into
     * one hypercall.
     */
    arch_start_context_switch(prev);

    /*
     * kernel -> kernel   lazy + transfer active
     *   user -> kernel   lazy + mmgrab() active
     *
     * kernel ->   user   switch + mmdrop() active
     *   user ->   user   switch
     */
    if (!next->mm) {                                // to kernel
        enter_lazy_tlb(prev->active_mm, next);

        next->active_mm = prev->active_mm;
        if (prev->mm)                           // from user
            mmgrab(prev->active_mm);
        else
            prev->active_mm = NULL;
    } else {                                        // to user
        membarrier_switch_mm(rq, prev->active_mm, next->mm);
        /*
         * sys_membarrier() requires an smp_mb() between setting
         * rq->curr / membarrier_switch_mm() and returning to userspace.
         *
         * The below provides this either through switch_mm(), or in
         * case 'prev->active_mm == next->mm' through
         * finish_task_switch()'s mmdrop().
         */
        switch_mm_irqs_off(prev->active_mm, next->mm, next);

        if (!prev->mm) {                        // from kernel
            /* will mmdrop() in finish_task_switch(). */
            rq->prev_mm = prev->active_mm;
            prev->active_mm = NULL;
        }
    }

    rq->clock_update_flags &= ~(RQCF_ACT_SKIP|RQCF_REQ_SKIP);

    prepare_lock_switch(rq, next, rf);

    /* Here we just switch the register state and the stack. */
    switch_to(prev, next, prev);
    barrier();

    return finish_task_switch(prev);
  }
```

Sau đó nó sẽ gọi hiện thực `switch_to` đặc thù theo kiến trúc để chuyển đổi trạng thái các thanh ghi và ngăn xếp nhân. Lưu ý rằng các thanh ghi được lưu lên ngăn xếp (stack) và con trỏ ngăn xếp (stack pointer) được lưu trong cấu trúc task:

```asm
#define switch_to(prev, next, last)               \
do {                                              \
    ((last) = __switch_to_asm((prev), (next)));   \
} while (0)


/*
 * %eax: prev task
 * %edx: next task
 */
.pushsection .text, "ax"
SYM_CODE_START(__switch_to_asm)
    /*
     * Save callee-saved registers
     * This must match the order in struct inactive_task_frame
     */
    pushl   %ebp
    pushl   %ebx
    pushl   %edi
    pushl   %esi
    /*
     * Flags are saved to prevent AC leakage. This could go
     * away if objtool would have 32bit support to verify
     * the STAC/CLAC correctness.
     */
    pushfl

    /* switch stack */
    movl    %esp, TASK_threadsp(%eax)
    movl    TASK_threadsp(%edx), %esp

  #ifdef CONFIG_STACKPROTECTOR
    movl    TASK_stack_canary(%edx), %ebx
    movl    %ebx, PER_CPU_VAR(stack_canary)+stack_canary_offset
  #endif

  #ifdef CONFIG_RETPOLINE
    /*
     * When switching from a shallower to a deeper call stack
     * the RSB may either underflow or use entries populated
     * with userspace addresses. On CPUs where those concerns
     * exist, overwrite the RSB with entries which capture
     * speculative execution to prevent attack.
     */
    FILL_RETURN_BUFFER %ebx, RSB_CLEAR_LOOPS, X86_FEATURE_RSB_CTXSW
    #endif

    /* Restore flags or the incoming task to restore AC state. */
    popfl
    /* restore callee-saved registers */
    popl    %esi
    popl    %edi
    popl    %ebx
    popl    %ebp

    jmp     __switch_to
  SYM_CODE_END(__switch_to_asm)
  .popsection
```

Bạn có thể nhận thấy rằng con trỏ lệnh (instruction pointer) **không** được lưu một cách tường minh. Điều này là không cần thiết vì:

- Một task sẽ luôn được tiếp tục (resume) trong chính hàm này.
- Địa chỉ trả về (return address) của bên gọi `schedule()` (hàm `context_switch()` luôn được inline) đã được lưu sẵn trên ngăn xếp nhân.
- Một lệnh `jmp` được dùng để thực thi `__switch_to()` — vốn là một hàm — và khi nó trả về, nó sẽ lấy (pop) địa chỉ trả về ban đầu (của task kế tiếp) ra khỏi ngăn xếp.

Screencast (trong bài gốc) dùng debugger đặt một breakpoint tại `__switch_to_asm` và xem xét ngăn xếp trong quá trình chuyển ngữ cảnh.

### Câu hỏi: chuyển ngữ cảnh

Ta đang thực hiện một lần chuyển ngữ cảnh. Hãy chọn tất cả các phát biểu **đúng**:

- thanh ghi ESP được lưu trong cấu trúc task
- thanh ghi EIP được lưu trong cấu trúc task
- các thanh ghi tổng quát (general registers) được lưu trong cấu trúc task
- thanh ghi ESP được lưu trên ngăn xếp (stack)
- thanh ghi EIP được lưu trên ngăn xếp
- các thanh ghi tổng quát được lưu trên ngăn xếp

---

## Chặn và đánh thức tác vụ (Blocking and waking up tasks)

### Các trạng thái của tác vụ (Task states)

Sơ đồ (trong bài gốc) thể hiện các trạng thái của tác vụ (luồng) và các phép chuyển trạng thái khả dĩ giữa chúng.

### Chặn luồng hiện tại (Blocking the current thread)

Chặn luồng hiện tại là một thao tác quan trọng cần thực hiện để hiện thực việc lập lịch tác vụ hiệu quả — ta muốn chạy các luồng khác trong khi các thao tác I/O đang hoàn tất.

Để thực hiện điều này, các thao tác sau diễn ra:

- Đặt trạng thái của luồng hiện tại thành `TASK_UNINTERRUPTIBLE` hoặc `TASK_INTERRUPTIBLE`
- Thêm task vào một hàng đợi chờ (waiting queue)
- Gọi bộ định thời (scheduler) — bộ này sẽ chọn ra một task mới từ hàng đợi READY
- Thực hiện chuyển ngữ cảnh sang task mới

Dưới đây là một số đoạn trích cho hiện thực của `wait_event`. Lưu ý rằng hàng đợi chờ là một danh sách (list) với một số thông tin bổ sung như con trỏ tới `task struct`.

Cũng cần lưu ý rằng rất nhiều nỗ lực được đặt vào việc đảm bảo không xảy ra deadlock giữa `wait_event` và `wake_up`: task được thêm vào danh sách **trước khi** kiểm tra điều kiện (condition), và các tín hiệu (signals) được kiểm tra **trước khi** gọi `schedule()`.

```c
/**
 * wait_event - sleep until a condition gets true
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 */
#define wait_event(wq_head, condition)            \
do {                                              \
  might_sleep();                                  \
  if (condition)                                  \
          break;                                  \
  __wait_event(wq_head, condition);               \
} while (0)

#define __wait_event(wq_head, condition)                                  \
    (void)___wait_event(wq_head, condition, TASK_UNINTERRUPTIBLE, 0, 0,   \
                        schedule())

/*
 * The below macro ___wait_event() has an explicit shadow of the __ret
 * variable when used from the wait_event_*() macros.
 *
 * This is so that both can use the ___wait_cond_timeout() construct
 * to wrap the condition.
 *
 * The type inconsistency of the wait_event_*() __ret variable is also
 * on purpose; we use long where we can return timeout values and int
 * otherwise.
 */
#define ___wait_event(wq_head, condition, state, exclusive, ret, cmd)    \
({                                                                       \
    __label__ __out;                                                     \
    struct wait_queue_entry __wq_entry;                                  \
    long __ret = ret;       /* explicit shadow */                        \
                                                                         \
    init_wait_entry(&__wq_entry, exclusive ? WQ_FLAG_EXCLUSIVE : 0);     \
    for (;;) {                                                           \
        long __int = prepare_to_wait_event(&wq_head, &__wq_entry, state);\
                                                                         \
        if (condition)                                                   \
            break;                                                       \
                                                                         \
        if (___wait_is_interruptible(state) && __int) {                  \
            __ret = __int;                                               \
            goto __out;                                                  \
        }                                                                \
                                                                         \
        cmd;                                                             \
    }                                                                    \
    finish_wait(&wq_head, &__wq_entry);                                  \
   __out:  __ret;                                                        \
 })

 void init_wait_entry(struct wait_queue_entry *wq_entry, int flags)
 {
    wq_entry->flags = flags;
    wq_entry->private = current;
    wq_entry->func = autoremove_wake_function;
    INIT_LIST_HEAD(&wq_entry->entry);
 }

 long prepare_to_wait_event(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry, int state)
 {
     unsigned long flags;
     long ret = 0;

     spin_lock_irqsave(&wq_head->lock, flags);
     if (signal_pending_state(state, current)) {
         /*
          * Exclusive waiter must not fail if it was selected by wakeup,
          * it should "consume" the condition we were waiting for.
          *
          * The caller will recheck the condition and return success if
          * we were already woken up, we can not miss the event because
          * wakeup locks/unlocks the same wq_head->lock.
          *
          * But we need to ensure that set-condition + wakeup after that
          * can't see us, it should wake up another exclusive waiter if
          * we fail.
          */
         list_del_init(&wq_entry->entry);
         ret = -ERESTARTSYS;
     } else {
         if (list_empty(&wq_entry->entry)) {
             if (wq_entry->flags & WQ_FLAG_EXCLUSIVE)
                 __add_wait_queue_entry_tail(wq_head, wq_entry);
             else
                 __add_wait_queue(wq_head, wq_entry);
         }
         set_current_state(state);
     }
     spin_unlock_irqrestore(&wq_head->lock, flags);

     return ret;
 }

 static inline void __add_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
 {
     list_add(&wq_entry->entry, &wq_head->head);
 }

 static inline void __add_wait_queue_entry_tail(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
 {
     list_add_tail(&wq_entry->entry, &wq_head->head);
 }

 /**
  * finish_wait - clean up after waiting in a queue
  * @wq_head: waitqueue waited on
  * @wq_entry: wait descriptor
  *
  * Sets current thread back to running state and removes
  * the wait descriptor from the given waitqueue if still
  * queued.
  */
 void finish_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
 {
     unsigned long flags;

     __set_current_state(TASK_RUNNING);
     /*
      * We can check for list emptiness outside the lock
      * IFF:
      *  - we use the "careful" check that verifies both
      *    the next and prev pointers, so that there cannot
      *    be any half-pending updates in progress on other
      *    CPU's that we haven't seen yet (and that might
      *    still change the stack area.
      * and
      *  - all other users take the lock (ie we can only
      *    have _one_ other CPU that looks at or modifies
      *    the list).
      */
     if (!list_empty_careful(&wq_entry->entry)) {
         spin_lock_irqsave(&wq_head->lock, flags);
         list_del_init(&wq_entry->entry);
         spin_unlock_irqrestore(&wq_head->lock, flags);
     }
 }
```

### Đánh thức một tác vụ (Waking up a task)

Ta có thể đánh thức các tác vụ bằng cách dùng nguyên thủy (primitive) `wake_up`. Các thao tác mức cao sau được thực hiện để đánh thức một tác vụ:

- Chọn một task từ hàng đợi chờ (waiting queue)
- Đặt trạng thái của task thành `TASK_READY`
- Chèn task vào hàng đợi READY của bộ định thời
- Trên hệ thống SMP (đa xử lý) đây là một thao tác phức tạp: mỗi bộ xử lý có hàng đợi riêng, các hàng đợi cần được cân bằng (balanced), và các CPU cần được báo hiệu (signaled)

```c
#define wake_up(x)                        __wake_up(x, TASK_NORMAL, 1, NULL)

/**
 * __wake_up - wake up threads blocked on a waitqueue.
 * @wq_head: the waitqueue
 * @mode: which threads
 * @nr_exclusive: how many wake-one or wake-many threads to wake up
 * @key: is directly passed to the wakeup function
 *
 * If this function wakes up a task, it executes a full memory barrier before
 * accessing the task state.
 */
void __wake_up(struct wait_queue_head *wq_head, unsigned int mode,
               int nr_exclusive, void *key)
{
    __wake_up_common_lock(wq_head, mode, nr_exclusive, 0, key);
}

static void __wake_up_common_lock(struct wait_queue_head *wq_head, unsigned int mode,
                  int nr_exclusive, int wake_flags, void *key)
{
  unsigned long flags;
  wait_queue_entry_t bookmark;

  bookmark.flags = 0;
  bookmark.private = NULL;
  bookmark.func = NULL;
  INIT_LIST_HEAD(&bookmark.entry);

  do {
          spin_lock_irqsave(&wq_head->lock, flags);
          nr_exclusive = __wake_up_common(wq_head, mode, nr_exclusive,
                                          wake_flags, key, &bookmark);
          spin_unlock_irqrestore(&wq_head->lock, flags);
  } while (bookmark.flags & WQ_FLAG_BOOKMARK);
}

/*
 * The core wakeup function. Non-exclusive wakeups (nr_exclusive == 0) just
 * wake everything up. If it's an exclusive wakeup (nr_exclusive == small +ve
 * number) then we wake all the non-exclusive tasks and one exclusive task.
 *
 * There are circumstances in which we can try to wake a task which has already
 * started to run but is not in state TASK_RUNNING. try_to_wake_up() returns
 * zero in this (rare) case, and we handle it by continuing to scan the queue.
 */
static int __wake_up_common(struct wait_queue_head *wq_head, unsigned int mode,
                            int nr_exclusive, int wake_flags, void *key,
                  wait_queue_entry_t *bookmark)
{
    wait_queue_entry_t *curr, *next;
    int cnt = 0;

    lockdep_assert_held(&wq_head->lock);

    if (bookmark && (bookmark->flags & WQ_FLAG_BOOKMARK)) {
          curr = list_next_entry(bookmark, entry);

          list_del(&bookmark->entry);
          bookmark->flags = 0;
    } else
          curr = list_first_entry(&wq_head->head, wait_queue_entry_t, entry);

    if (&curr->entry == &wq_head->head)
          return nr_exclusive;

    list_for_each_entry_safe_from(curr, next, &wq_head->head, entry) {
          unsigned flags = curr->flags;
          int ret;

          if (flags & WQ_FLAG_BOOKMARK)
                  continue;

          ret = curr->func(curr, mode, wake_flags, key);
          if (ret < 0)
                  break;
          if (ret && (flags & WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
                  break;

          if (bookmark && (++cnt > WAITQUEUE_WALK_BREAK_CNT) &&
                          (&next->entry != &wq_head->head)) {
                  bookmark->flags = WQ_FLAG_BOOKMARK;
                  list_add_tail(&bookmark->entry, &next->entry);
                  break;
          }
    }

    return nr_exclusive;
}

int autoremove_wake_function(struct wait_queue_entry *wq_entry, unsigned mode, int sync, void *key)
{
    int ret = default_wake_function(wq_entry, mode, sync, key);

    if (ret)
        list_del_init_careful(&wq_entry->entry);

    return ret;
}

int default_wake_function(wait_queue_entry_t *curr, unsigned mode, int wake_flags,
                    void *key)
{
    WARN_ON_ONCE(IS_ENABLED(CONFIG_SCHED_DEBUG) && wake_flags & ~WF_SYNC);
    return try_to_wake_up(curr->private, mode, wake_flags);
}

/**
 * try_to_wake_up - wake up a thread
 * @p: the thread to be awakened
 * @state: the mask of task states that can be woken
 * @wake_flags: wake modifier flags (WF_*)
 *
 * Conceptually does:
 *
 *   If (@state & @p->state) @p->state = TASK_RUNNING.
 *
 * If the task was not queued/runnable, also place it back on a runqueue.
 *
 * This function is atomic against schedule() which would dequeue the task.
 *
 * It issues a full memory barrier before accessing @p->state, see the comment
 * with set_current_state().
 *
 * Uses p->pi_lock to serialize against concurrent wake-ups.
 *
 * Relies on p->pi_lock stabilizing:
 *  - p->sched_class
 *  - p->cpus_ptr
 *  - p->sched_task_group
 * in order to do migration, see its use of select_task_rq()/set_task_cpu().
 *
 * Tries really hard to only take one task_rq(p)->lock for performance.
 * Takes rq->lock in:
 *  - ttwu_runnable()    -- old rq, unavoidable, see comment there;
 *  - ttwu_queue()       -- new rq, for enqueue of the task;
 *  - psi_ttwu_dequeue() -- much sadness :-( accounting will kill us.
 *
 * As a consequence we race really badly with just about everything. See the
 * many memory barriers and their comments for details.
 *
 * Return: %true if @p->state changes (an actual wakeup was done),
 *           %false otherwise.
 */
 static int
 try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
 {
     ...
```

---

## Trưng dụng (chiếm quyền) tác vụ (Preempting tasks)

Cho tới điểm này, ta đã xem xét cách các lần chuyển ngữ cảnh diễn ra một cách **tự nguyện (voluntary)** giữa các luồng. Tiếp theo, ta sẽ xem cách xử lý việc **trưng dụng (preemption)**. Ta sẽ bắt đầu với trường hợp đơn giản hơn — nhân được cấu hình **không trưng dụng (non preemptive)** — rồi chuyển sang trường hợp nhân **có trưng dụng (preemptive)**.

### Nhân không trưng dụng (Non preemptive kernel)

- Tại mỗi nhịp (tick), nhân kiểm tra xem tiến trình hiện tại đã dùng hết lượng thời gian (time slice) của nó hay chưa.
- Nếu điều đó xảy ra, một cờ (flag) được đặt trong ngữ cảnh ngắt (interrupt context).
- Trước khi quay về không gian người dùng, nhân kiểm tra cờ này và gọi `schedule()` nếu cần.
- Trong trường hợp này, các task **không** bị trưng dụng khi đang chạy ở chế độ nhân (ví dụ trong một lời gọi hệ thống), nên không có vấn đề về đồng bộ hóa (synchronization).

### Nhân có trưng dụng (Preemptive kernel)

Trong trường hợp này, task hiện tại có thể bị trưng dụng **ngay cả khi** ta đang chạy ở chế độ nhân và đang thực thi một lời gọi hệ thống. Điều này đòi hỏi phải dùng các nguyên thủy đồng bộ hóa đặc biệt: `preempt_disable` và `preempt_enable`.

Để đơn giản hóa việc xử lý cho nhân có trưng dụng — và vì các nguyên thủy đồng bộ hóa vốn dĩ đã cần cho trường hợp SMP — việc trưng dụng được tự động vô hiệu hóa khi một spinlock được sử dụng.

Như trước, nếu ta gặp một điều kiện đòi hỏi phải trưng dụng task hiện tại (time slice của nó đã hết), một cờ được đặt. Cờ này được kiểm tra mỗi khi việc trưng dụng được kích hoạt lại, ví dụ khi thoát khỏi một vùng tới hạn (critical section) thông qua `spin_unlock()`, và nếu cần, bộ định thời được gọi để chọn một task mới.

---

## Ngữ cảnh tiến trình (Process context)

Giờ đây, sau khi đã khảo sát hiện thực của tiến trình và luồng (task), cách chuyển ngữ cảnh diễn ra, cách chặn / đánh thức / trưng dụng các task, cuối cùng ta có thể định nghĩa **ngữ cảnh tiến trình (process context)** là gì và nó có những thuộc tính nào:

- Nhân đang thực thi trong **ngữ cảnh tiến trình** khi nó đang chạy một lời gọi hệ thống.
- Trong ngữ cảnh tiến trình có một ngữ cảnh được xác định rõ ràng, và ta có thể truy cập dữ liệu của tiến trình hiện tại bằng `current`.
- Trong ngữ cảnh tiến trình, ta có thể **ngủ (sleep)** (chờ trên một điều kiện).
- Trong ngữ cảnh tiến trình, ta có thể truy cập không gian người dùng (user-space) — trừ khi ta đang chạy trong ngữ cảnh của một luồng nhân (kernel thread).

### Luồng nhân (Kernel threads)

Đôi khi phần lõi của nhân hoặc các trình điều khiển thiết bị (device drivers) cần thực hiện các thao tác có thể bị chặn (blocking), và do đó chúng cần chạy trong ngữ cảnh tiến trình.

**Luồng nhân (kernel threads)** được dùng đúng cho mục đích này; chúng là một lớp tác vụ đặc biệt không có các tài nguyên "không gian người dùng" (ví dụ: không có không gian địa chỉ hay tập tin đang mở).

Screencast (trong bài gốc) xem xét kỹ hơn về các luồng nhân.

---

## Dùng các kịch bản gdb để khảo sát nhân (Using gdb scripts for kernel inspection)

Nhân Linux đi kèm một tập hợp các lệnh gdb mở rộng được định nghĩa sẵn mà ta có thể dùng để khảo sát nhân trong quá trình gỡ lỗi. Chúng sẽ tự động được nạp miễn là tệp `gdbinit` được thiết lập đúng:

```
ubuntu@so2:/linux/tools/labs$ cat ~/.gdbinit
add-auto-load-safe-path /linux/scripts/gdb/vmlinux-gdb.py
```

Tất cả các lệnh đặc thù của nhân đều có tiền tố `lx-`. Bạn có thể dùng phím TAB trong gdb để liệt kê tất cả:

```
(gdb) lx-
lx-clk-summary        lx-dmesg              lx-mounts
lx-cmdline            lx-fdtdump            lx-ps
lx-configdump         lx-genpd-summary      lx-symbols
lx-cpus               lx-iomem              lx-timerlist
lx-device-list-bus    lx-ioports            lx-version
lx-device-list-class  lx-list-check
lx-device-list-tree   lx-lsmod
```

Hiện thực của các lệnh này có thể được tìm thấy ở `script/gdb/linux`. Hãy xem xét kỹ hơn hiện thực của `lx-ps`:

```python
task_type = utils.CachedType("struct task_struct")


def task_lists():
 task_ptr_type = task_type.get_type().pointer()
 init_task = gdb.parse_and_eval("init_task").address
 t = g = init_task

 while True:
     while True:
         yield t

         t = utils.container_of(t['thread_group']['next'],
                                task_ptr_type, "thread_group")
         if t == g:
             break

     t = g = utils.container_of(g['tasks']['next'],
                                task_ptr_type, "tasks")
     if t == init_task:
         return


 class LxPs(gdb.Command):
 """Dump Linux tasks."""

 def __init__(self):
     super(LxPs, self).__init__("lx-ps", gdb.COMMAND_DATA)

 def invoke(self, arg, from_tty):
     gdb.write("{:>10} {:>12} {:>7}\n".format("TASK", "PID", "COMM"))
     for task in task_lists():
         gdb.write("{} {:^5} {}\n".format(
             task.format_string().split()[0],
             task["pid"].format_string(),
             task["comm"].string()))
```

### Câu hỏi: Kịch bản gdb của nhân

Thay đổi sau đối với kịch bản `lx-ps` đang cố gắng đạt được điều gì?

```diff
diff --git a/scripts/gdb/linux/tasks.py b/scripts/gdb/linux/tasks.py
index 17ec19e9b5bf..7e43c163832f 100644
--- a/scripts/gdb/linux/tasks.py
+++ b/scripts/gdb/linux/tasks.py
@@ -75,10 +75,13 @@ class LxPs(gdb.Command):
     def invoke(self, arg, from_tty):
         gdb.write("{:>10} {:>12} {:>7}\n".format("TASK", "PID", "COMM"))
         for task in task_lists():
-            gdb.write("{} {:^5} {}\n".format(
+            check = task["mm"].format_string() == "0x0"
+            gdb.write("{} {:^5} {}{}{}\n".format(
                 task.format_string().split()[0],
                 task["pid"].format_string(),
-                task["comm"].string()))
+                "[" if check else "",
+                task["comm"].string(),
+                "]" if check else ""))


 LxPs()
```

> *Gợi ý:* trường `mm` bằng `0x0` (NULL) là dấu hiệu của một **luồng nhân (kernel thread)** — vốn không có không gian địa chỉ người dùng. Thay đổi này bao tên các luồng nhân trong dấu ngoặc vuông `[...]`, tương tự cách `ps` hiển thị chúng.

---

*— Hết bản dịch phần "Processes" —*
