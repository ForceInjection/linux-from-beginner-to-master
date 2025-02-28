# Linux 进程间共享内存通信方案 v2：基于 Futex 锁的实现

`Futex`（`Fast Userspace Mutex`）是 `Linux` 系统中实现高效同步机制的核心技术，其设计目标是通过用户态与内核态协同工作来最小化系统调用开销。

详细参考：[**一文搞清楚操作系统中的锁**](https://mp.weixin.qq.com/s/2H1-7Iiuxyp3uWNEItqJ8g)

## 相关代码

### [common_v2.h](common_v2.h)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdatomic.h>

#define SHM_NAME "/demo_shm"  // 共享内存名称
#define SEM_AVAIL "/sem_avail"  // 可用信号量名称
#define SEM_DONE "/sem_done"  // 完成信号量名称
#define INIT_SIZE 4096  // 共享内存初始大小

// 共享内存头结构体
struct shm_header {
    int lock;  // futex锁变量
    size_t capacity;  // 共享内存容量
    size_t data_size;  // 数据大小
    volatile int ready_flag;  // 数据就绪标志
    char data[];  // 数据存储区域
};

// futex系统调用帮助函数
static inline int futex(int *uaddr, int futex_op, int val,
                        const struct timespec *timeout,
                        int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}
```

### [process_a_v2.c](process_a_v2.c)

```c
#include "common_v2.h"

int main() {
    // 创建共享内存对象，O_CREAT表示创建，O_RDWR表示读写权限
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");  // 创建共享内存失败
        exit(EXIT_FAILURE);
    }

    // 设置共享内存大小为初始大小
    if (ftruncate(shm_fd, INIT_SIZE) == -1) {
        perror("ftruncate failed");  // 设置大小失败
        exit(EXIT_FAILURE);
    }

    // 将共享内存映射到当前进程的地址空间
    struct shm_header *header = mmap(NULL, INIT_SIZE,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shm_fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap failed");  // 内存映射失败
        exit(EXIT_FAILURE);
    }

    // 初始化futex锁，初始值为0
    atomic_store((_Atomic int *)&header->lock, 0);

    // 示例数据
    const char *messages[] = {"Hello", "World", "Dynamic", "Memory"};

    // 创建信号量
    sem_unlink(SEM_AVAIL);  // 删除已存在的信号量（如果有的话）
    sem_unlink(SEM_DONE);

    sem_t *sem_avail = sem_open(SEM_AVAIL, O_CREAT | O_EXCL, 0666, 0);
    if (sem_avail == SEM_FAILED) {
        perror("sem_open failed for SEM_AVAIL");  // 创建信号量失败
        exit(EXIT_FAILURE);
    }

    sem_t *sem_done = sem_open(SEM_DONE, O_CREAT | O_EXCL, 0666, 0);
    if (sem_done == SEM_FAILED) {
        perror("sem_open failed for SEM_DONE");  // 创建信号量失败
        exit(EXIT_FAILURE);
    }

    // 循环发送数据
    for (int i = 0; i < 4; i++) {
        size_t data_len = strlen(messages[i]) + 1;  // 数据长度（包括\0）

        // 获取futex锁
        while (atomic_exchange((_Atomic int *)&header->lock, 1)) {
            futex((int *)&header->lock, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
        }

        // 检查共享内存大小是否足够，不足则扩展
        if (data_len > (header->capacity - sizeof(struct shm_header))) {
            size_t new_cap = header->capacity * 2;
            while (new_cap < (sizeof(struct shm_header) + data_len)) {
                new_cap *= 2;
            }

            munmap(header, header->capacity);  // 解除当前映射
            if (ftruncate(shm_fd, new_cap) == -1) {
                perror("ftruncate failed during resizing");  // 扩展大小失败
                exit(EXIT_FAILURE);
            }

            header = mmap(NULL, new_cap,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_fd, 0);
            if (header == MAP_FAILED) {
                perror("mmap failed after resizing");  // 重新映射失败
                exit(EXIT_FAILURE);
            }

            header->capacity = new_cap;  // 更新容量
        }

        // 写入数据
        memcpy(header->data, messages[i], data_len);  // 将消息复制到共享内存
        header->data_size = data_len;  // 设置数据大小
        header->ready_flag = 1;  // 设置数据就绪标志

        // 释放锁
        atomic_store((_Atomic int *)&header->lock, 0);
        futex((int *)&header->lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);

        printf("[A] Sent: %s\n", messages[i]);  // 输出已发送数据

        // 通知进程B有数据可读取
        sem_post(sem_avail);

        // 等待进程B确认数据已被处理
        sem_wait(sem_done);
    }

    // 清理资源
    munmap(header, header->capacity);
    close(shm_fd);
    sem_close(sem_avail);
    sem_close(sem_done);
    sem_unlink(SEM_AVAIL);
    sem_unlink(SEM_DONE);

    return 0;
}
```

### [process_b_v2.c](process_b_v2.c)

```c
#include "common_v2.h"

int main() {
    // 打开共享内存对象，以只读写模式
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");  // 打开共享内存失败
        exit(EXIT_FAILURE);
    }

    // 获取共享内存的元数据（如大小等）
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("fstat failed");  // 获取元数据失败
        exit(EXIT_FAILURE);
    }

    // 将共享内存映射到当前进程的地址空间
    struct shm_header *header = mmap(NULL, shm_stat.st_size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shm_fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap failed");  // 内存映射失败
        exit(EXIT_FAILURE);
    }

    // 打开由进程A创建的信号量，用于同步操作
    sem_t *sem_avail = sem_open(SEM_AVAIL, 0);
    if (sem_avail == SEM_FAILED) {
        perror("sem_open failed for SEM_AVAIL");  // 打开信号量失败
        exit(EXIT_FAILURE);
    }

    sem_t *sem_done = sem_open(SEM_DONE, 0);
    if (sem_done == SEM_FAILED) {
        perror("sem_open failed for SEM_DONE");  // 打开信号量失败
        exit(EXIT_FAILURE);
    }

    // 循环接收数据
    for (int i = 0; i < 4; i++) {
        // 等待信号量，表示进程A有数据可读取
        sem_wait(sem_avail);

        // 使用futex实现互斥锁，确保线程安全
        while (atomic_exchange((_Atomic int *)&header->lock, 1)) {
            // 如果锁被占用，等待futex信号
            futex((int *)&header->lock, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
        }

        // 检查共享内存大小是否变化
        if (fstat(shm_fd, &shm_stat) == -1) {
            perror("fstat failed");  // 获取元数据失败
            exit(EXIT_FAILURE);
        }

        // 如果共享内存增长，重新映射
        if (shm_stat.st_size > header->capacity) {
            munmap(header, header->capacity);  // 解除当前映射
            header = mmap(NULL, shm_stat.st_size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd, 0);
            if (header == MAP_FAILED) {
                perror("mmap failed after resizing");  // 重新映射失败
                exit(EXIT_FAILURE);
            }
        }

        // 读取数据
        if (header->ready_flag) {
            char buf[256];  // 临时缓冲区
            memcpy(buf, header->data, header->data_size);  // 复制数据
            header->ready_flag = 0;  // 重置标志
            printf("[B] Received: %s\n", buf);  // 输出数据
        }

        // 释放锁并唤醒等待线程
        atomic_store((_Atomic int *)&header->lock, 0);
        futex((int *)&header->lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);

        // 通知进程A数据已被处理
        sem_post(sem_done);
    }

    // 清理资源
    munmap(header, shm_stat.st_size);
    close(shm_fd);
    sem_close(sem_avail);
    sem_close(sem_done);

    return 0;
}
```

## 编译及运行

### GCC

```bash
gcc -o process_a_v2 process_a_v2.c -pthread
gcc -o process_b_v2 process_b_v2.c -pthread
```

`-pthread` 参数在 `GCC` 编译时用于启用 `POSIX` 线程支持，主要原因如下：

* **线程库链接**：`-pthread` 参数告诉编译器和链接器需要使用 `POSIX` 线程库（`libpthread`）。这个库提供了线程相关的函数和数据结构，如 `pthread_*`，`sem_*` 等。
* **头文件包含**：使用 `-pthread` 参数后，编译器会自动包含额外的头文件，这些头文件定义了与线程相关的 `API`，如 `<pthread.h>`。
* **链接器选项**：`-pthread` 参数会自动将 `pthread` 库链接到目标程序中，确保代码中使用的线程相关函数（如 `pthread_create`、`sem_init`）能找到对应的实现。
* **线程安全**：这个参数还会启用编译器对线程安全的优化，确保在多线程环境下代码的正确性。

例如，在代码中使用了 `pthread` 或者 `sem` 相关的 `API` 时（我们使用了 `sem`），必须使用 `-pthread` 参数，否则会报未定义符号的错误。

### 执行

>注意：执行有先后顺序

**terminal #1**:

```bash
./process_a_v2
[A] Sent: Hello
[A] Sent: World
[A] Sent: Dynamic
[A] Sent: Memory
```

**termianl #2**:

```bash
./process_b_v2
[B] Received: Hello
[B] Received: World
[B] Received: Dynamic
[B] Received: Memory
```

### 追踪执行过程（strace ./process_a_v2）

```bash
strace ./process_a_v2
execve("./process_a_v2", ["./process_a_v2"], 0x7ffd9f428350 /* 25 vars */) = 0
brk(NULL)                               = 0x5f96e83ea000
arch_prctl(0x3001 /* ARCH_??? */, 0x7fff03f73db0) = -1 EINVAL (Invalid argument)
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7dc18b223000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=49907, ...}, AT_EMPTY_PATH) = 0
mmap(NULL, 49907, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7dc18b216000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0P\237\2\0\0\0\0\0"..., 832) = 832
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
pread64(3, "\4\0\0\0 \0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0"..., 48, 848) = 48
pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\315A\vq\17\17\tLh2\355\331Y1\0m"..., 68, 896) = 68
newfstatat(3, "", {st_mode=S_IFREG|0755, st_size=2220400, ...}, AT_EMPTY_PATH) = 0
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
mmap(NULL, 2264656, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7dc18ae00000
mprotect(0x7dc18ae28000, 2023424, PROT_NONE) = 0
mmap(0x7dc18ae28000, 1658880, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x28000) = 0x7dc18ae28000
mmap(0x7dc18afbd000, 360448, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1bd000) = 0x7dc18afbd000
mmap(0x7dc18b016000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x215000) = 0x7dc18b016000
mmap(0x7dc18b01c000, 52816, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7dc18b01c000
close(3)                                = 0
mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7dc18b213000
arch_prctl(ARCH_SET_FS, 0x7dc18b213740) = 0
set_tid_address(0x7dc18b213a10)         = 72191
set_robust_list(0x7dc18b213a20, 24)     = 0
rseq(0x7dc18b2140e0, 0x20, 0, 0x53053053) = 0
mprotect(0x7dc18b016000, 16384, PROT_READ) = 0
mprotect(0x5f96e739b000, 4096, PROT_READ) = 0
mprotect(0x7dc18b25d000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
munmap(0x7dc18b216000, 49907)           = 0
openat(AT_FDCWD, "/dev/shm/demo_shm", O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC, 0666) = 3
ftruncate(3, 4096)                      = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x7dc18b25c000
unlink("/dev/shm/sem.sem_avail")        = -1 ENOENT (No such file or directory)
unlink("/dev/shm/sem.sem_done")         = -1 ENOENT (No such file or directory)
getrandom("\x4b\xc1\x49\x29\x0d\x56\x88\x4b", 8, GRND_NONBLOCK) = 8
newfstatat(AT_FDCWD, "/dev/shm/sem.p8PWrE", 0x7fff03f73a80, AT_SYMLINK_NOFOLLOW) = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/dev/shm/sem.p8PWrE", O_RDWR|O_CREAT|O_EXCL, 0666) = 4
write(4, "\0\0\0\0\0\0\0\0\200\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 32) = 32
mmap(NULL, 32, PROT_READ|PROT_WRITE, MAP_SHARED, 4, 0) = 0x7dc18b222000
link("/dev/shm/sem.p8PWrE", "/dev/shm/sem.sem_avail") = 0
newfstatat(4, "", {st_mode=S_IFREG|0644, st_size=32, ...}, AT_EMPTY_PATH) = 0
getrandom("\x3a\x6e\xbf\x75\xe4\x3b\x8b\x0b", 8, GRND_NONBLOCK) = 8
brk(NULL)                               = 0x5f96e83ea000
brk(0x5f96e840b000)                     = 0x5f96e840b000
unlink("/dev/shm/sem.p8PWrE")           = 0
close(4)                                = 0
getrandom("\xaa\x37\x39\x49\xa1\x3d\xa6\xf7", 8, GRND_NONBLOCK) = 8
getrandom("\x4a\x90\x58\xc3\x3d\xf1\xb9\x19", 8, GRND_NONBLOCK) = 8
newfstatat(AT_FDCWD, "/dev/shm/sem.60EknS", 0x7fff03f73a80, AT_SYMLINK_NOFOLLOW) = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/dev/shm/sem.60EknS", O_RDWR|O_CREAT|O_EXCL, 0666) = 4
write(4, "\0\0\0\0\0\0\0\0\200\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 32) = 32
mmap(NULL, 32, PROT_READ|PROT_WRITE, MAP_SHARED, 4, 0) = 0x7dc18b221000
link("/dev/shm/sem.60EknS", "/dev/shm/sem.sem_done") = 0
newfstatat(4, "", {st_mode=S_IFREG|0644, st_size=32, ...}, AT_EMPTY_PATH) = 0
unlink("/dev/shm/sem.60EknS")           = 0
close(4)                                = 0
futex(0x7dc18b25c000, FUTEX_WAKE_PRIVATE, 1) = 0
newfstatat(1, "", {st_mode=S_IFCHR|0600, st_rdev=makedev(0x88, 0x1), ...}, AT_EMPTY_PATH) = 0
write(1, "[A] Sent: Hello\n", 16[A] Sent: Hello
)       = 16
futex(0x7dc18b221000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
futex(0x7dc18b25c000, FUTEX_WAKE_PRIVATE, 1) = 0
write(1, "[A] Sent: World\n", 16[A] Sent: World
)       = 16
futex(0x7dc18b222000, FUTEX_WAKE, 1)    = 1
futex(0x7dc18b221000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
futex(0x7dc18b25c000, FUTEX_WAKE_PRIVATE, 1) = 0
write(1, "[A] Sent: Dynamic\n", 18[A] Sent: Dynamic
)     = 18
futex(0x7dc18b222000, FUTEX_WAKE, 1)    = 1
futex(0x7dc18b221000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
futex(0x7dc18b25c000, FUTEX_WAKE_PRIVATE, 1) = 0
write(1, "[A] Sent: Memory\n", 17[A] Sent: Memory
)      = 17
futex(0x7dc18b222000, FUTEX_WAKE, 1)    = 1
futex(0x7dc18b221000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
munmap(0x7dc18b25c000, 0)               = -1 EINVAL (Invalid argument)
close(3)                                = 0
munmap(0x7dc18b222000, 32)              = 0
munmap(0x7dc18b221000, 32)              = 0
unlink("/dev/shm/sem.sem_avail")        = 0
unlink("/dev/shm/sem.sem_done")         = 0
exit_group(0)                           = ?
+++ exited with 0 +++
```

**关键调用说明**：

#### **1. 进程启动和初始化**

```bash
execve("./process_a_v2", ["./process_a_v2"], 0x7ffd9f428350 /* 25 vars */) = 0
```

* **解析**：进程 `A` 通过 `execve` 系统调用启动;
* **关键点**：`execve` 是进程执行的起点，加载程序 `process_a_v2` 并开始运行。

#### **2. 内存分配**

```bash
brk(NULL)                               = 0x5f96e83ea000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7dc18b223000
```

* **解析**：
  * `brk(NULL)` 获取当前进程的内存数据段末尾地址。
  * `mmap` 分配了 **8KB** 的匿名内存（`MAP_PRIVATE|MAP_ANONYMOUS`），用于存储动态库加载信息或其他数据。

#### **3. 动态链接器加载**

```bash
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
mmap(NULL, 49907, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7dc18b216000
```

* **解析**：
  * 检查 `/etc/ld.so.preload` 是否存在（不存在）。
  * 打开 `/etc/ld.so.cache` 并映射到内存，用于加速动态链接。

#### **4. 加载 `libc.so.6`**

```bash
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
mmap(NULL, 2264656, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7dc18ae00000
```

* **解析**：
  * 打开 `libc.so.6` 并映射到内存，这是 C 标准库，提供了 `memcpy`、`printf` 等函数。

#### **5. 创建共享内存**

```c
openat(AT_FDCWD, "/dev/shm/demo_shm", O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC, 0666) = 3
ftruncate(3, 4096)                      = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x7dc18b25c000
```

* **解析**：
  * 进程 `A` 创建并打开共享内存对象 `/dev/shm/demo_shm`。
  * 使用 `ftruncate` 设置共享内存的大小为 **4096** 字节。
  * 使用 `mmap` 将共享内存映射到进程地址空间，地址为 `0x7dc18b25c000`。

#### **6. 初始化信号量**

```bash
unlink("/dev/shm/sem.sem_avail")        = -1 ENOENT (No such file or directory)
unlink("/dev/shm/sem.sem_done")         = -1 ENOENT (No such file or directory)
getrandom("\x4b\xc1\x49\x29\x0d\x56\x88\x4b", 8, GRND_NONBLOCK) = 8
```

* **解析**：
  * 使用 `unlink` 尝试删除已存在的信号量（如果存在）。
  * 使用 `getrandom` 获取随机种子，用于信号量的初始化。

#### **7. 发送数据**

```bash
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x7dc18b25c000
memcpy(header->data, messages[i], data_len);
write(1, "[A] Sent: Hello\n", 16[A] Sent: Hello
)       = 16
sem_post(sem_avail) // 映射到底层的 futex 调用
futex(0x7dc18b222000, FUTEX_WAKE, 1)    = 1
```

* **解析**：
  * `mmap` 将共享内存映射到地址 `0x7dc18b25c000`。
  * 进程 `A` 将数据（如 `"Hello"`）写入共享内存的 `data` 区域。
  * 使用 `sem_post` 通知进程 `B` 数据已准备好（通过信号量 `SEM_AVAIL`）。

#### **8. 等待同步**

```bash
futex(0x7dc18b221000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
```

* **解析**：
  * 进程A 调用 `futex` 进入等待状态，等待进程 `B` 处理完数据并通过信号量 `SEM_DONE` 通知。

#### **9. 清理资源**

```bash
munmap(0x7dc18b25c000, 0)               = -1 EINVAL (Invalid argument)
close(3)                                = 0
munmap(0x7dc18b222000, 32)              = 0
munmap(0x7dc18b221000, 32)              = 0
unlink("/dev/shm/sem.sem_avail")        = 0
unlink("/dev/shm/sem.sem_done")         = 0
```

* **解析**：
  * 使用 `munmap` 解除共享内存映射。
  * 使用 `close` 关闭共享内存文件描述符。
  * 使用 `unlink` 删除信号量。

### strace ./process_b_v2

```bash
execve("./process_b_v2", ["./process_b_v2"], 0x7ffd4aa58ad0 /* 25 vars */) = 0
brk(NULL)                               = 0x62cd5db4f000
arch_prctl(0x3001 /* ARCH_??? */, 0x7ffc609554d0) = -1 EINVAL (Invalid argument)
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x725786dd4000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=49907, ...}, AT_EMPTY_PATH) = 0
mmap(NULL, 49907, PROT_READ, MAP_PRIVATE, 3, 0) = 0x725786dc7000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0P\237\2\0\0\0\0\0"..., 832) = 832
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
pread64(3, "\4\0\0\0 \0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0"..., 48, 848) = 48
pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\315A\vq\17\17\tLh2\355\331Y1\0m"..., 68, 896) = 68
newfstatat(3, "", {st_mode=S_IFREG|0755, st_size=2220400, ...}, AT_EMPTY_PATH) = 0
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
mmap(NULL, 2264656, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x725786a00000
mprotect(0x725786a28000, 2023424, PROT_NONE) = 0
mmap(0x725786a28000, 1658880, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x28000) = 0x725786a28000
mmap(0x725786bbd000, 360448, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1bd000) = 0x725786bbd000
mmap(0x725786c16000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x215000) = 0x725786c16000
mmap(0x725786c1c000, 52816, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x725786c1c000
close(3)                                = 0
mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x725786dc4000
arch_prctl(ARCH_SET_FS, 0x725786dc4740) = 0
set_tid_address(0x725786dc4a10)         = 72195
set_robust_list(0x725786dc4a20, 24)     = 0
rseq(0x725786dc50e0, 0x20, 0, 0x53053053) = 0
mprotect(0x725786c16000, 16384, PROT_READ) = 0
mprotect(0x62cd5c162000, 4096, PROT_READ) = 0
mprotect(0x725786e0e000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
munmap(0x725786dc7000, 49907)           = 0
openat(AT_FDCWD, "/dev/shm/demo_shm", O_RDWR|O_NOFOLLOW|O_CLOEXEC) = 3
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786e0d000
openat(AT_FDCWD, "/dev/shm/sem.sem_avail", O_RDWR|O_NOFOLLOW) = 4
newfstatat(4, "", {st_mode=S_IFREG|0644, st_size=32, ...}, AT_EMPTY_PATH) = 0
getrandom("\xac\x23\xd4\xb7\xd4\x12\xcf\x42", 8, GRND_NONBLOCK) = 8
brk(NULL)                               = 0x62cd5db4f000
brk(0x62cd5db70000)                     = 0x62cd5db70000
mmap(NULL, 32, PROT_READ|PROT_WRITE, MAP_SHARED, 4, 0) = 0x725786dd3000
close(4)                                = 0
openat(AT_FDCWD, "/dev/shm/sem.sem_done", O_RDWR|O_NOFOLLOW) = 4
newfstatat(4, "", {st_mode=S_IFREG|0644, st_size=32, ...}, AT_EMPTY_PATH) = 0
mmap(NULL, 32, PROT_READ|PROT_WRITE, MAP_SHARED, 4, 0) = 0x725786dd2000
close(4)                                = 0
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
munmap(0x725786e0d000, 0)               = -1 EINVAL (Invalid argument)
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786dd1000
newfstatat(1, "", {st_mode=S_IFCHR|0600, st_rdev=makedev(0x88, 0), ...}, AT_EMPTY_PATH) = 0
write(1, "[B] Received: Hello\n", 20[B] Received: Hello
)   = 20
futex(0x725786dd1000, FUTEX_WAKE_PRIVATE, 1) = 0
futex(0x725786dd2000, FUTEX_WAKE, 1)    = 1
futex(0x725786dd3000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
munmap(0x725786dd1000, 0)               = -1 EINVAL (Invalid argument)
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786dd0000
write(1, "[B] Received: World\n", 20[B] Received: World
)   = 20
futex(0x725786dd0000, FUTEX_WAKE_PRIVATE, 1) = 0
futex(0x725786dd2000, FUTEX_WAKE, 1)    = 1
futex(0x725786dd3000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
munmap(0x725786dd0000, 0)               = -1 EINVAL (Invalid argument)
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786dcf000
write(1, "[B] Received: Dynamic\n", 22[B] Received: Dynamic
) = 22
futex(0x725786dcf000, FUTEX_WAKE_PRIVATE, 1) = 0
futex(0x725786dd2000, FUTEX_WAKE, 1)    = 1
futex(0x725786dd3000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
munmap(0x725786dcf000, 0)               = -1 EINVAL (Invalid argument)
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786dce000
write(1, "[B] Received: Memory\n", 21[B] Received: Memory
)  = 21
futex(0x725786dce000, FUTEX_WAKE_PRIVATE, 1) = 0
futex(0x725786dd2000, FUTEX_WAKE, 1)    = 1
munmap(0x725786dce000, 4096)            = 0
close(3)                                = 0
munmap(0x725786dd3000, 32)              = 0
munmap(0x725786dd2000, 32)              = 0
exit_group(0)                           = ?
+++ exited with 0 +++
```

 **关键调用说明**：

#### **1. 进程启动和初始化**

```c
execve("./process_b_v2", ["./process_b_v2"], 0x7ffd4aa58ad0 /* 25 vars */) = 0
```

* **解析**：进程 `B` 通过 `execve` 系统调用启动，加载程序 `process_b_v2` 并开始运行。

* **关键点**：`execve` 是进程执行的起点，为后续操作做好准备。

#### **2. 内存分配**

```c
brk(NULL)                               = 0x62cd5db4f000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x725786dd4000
```

* **解析**：
  * `brk(NULL)` 获取当前进程的内存数据段末尾地址。
  * `mmap` 分配了 **8KB** 的匿名内存，用于动态库加载或其他数据存储。

#### **3. 动态链接器加载**

```c
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
mmap(NULL, 49907, PROT_READ, MAP_PRIVATE, 3, 0) = 0x725786dc7000
```

* **解析**：
  * 查找 `/etc/ld.so.preload` 文件（不存在）。
  * 打开 `/etc/ld.so.cache` 并映射到内存，用于加速动态链接。

#### **4. 加载 `libc.so.6`**

```c
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
mmap(NULL, 2264656, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x725786a00000
```

* **解析**：
  * 打开 `libc.so.6` 并映射到内存，C 标准库的函数（如 `memcpy`、`printf`）依赖于它。

#### **5. 打开共享内存**

```c
openat(AT_FDCWD, "/dev/shm/demo_shm", O_RDWR|O_NOFOLLOW|O_CLOEXEC) = 3
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786e0d000
```

* **解析**：
  * 进程 `B` 以读写模式打开共享内存对象 `/dev/shm/demo_shm`。
  * 使用 `mmap` 将共享内存映射到进程地址空间。

#### **6. 打开信号量**

```c
openat(AT_FDCWD, "/dev/shm/sem.sem_avail", O_RDWR|O_NOFOLLOW) = 4
mmap(NULL, 32, PROT_READ|PROT_WRITE, MAP_SHARED, 4, 0) = 0x725786dd3000
close(4)                                = 0
```

* **解析**：
  * 打开信号量 `SEM_AVAIL` 并映射到内存，用于接收进程 `A` 的数据通知。

#### **7. 循环接收数据**

```c
futex(0x725786dd3000, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 0, NULL, FUTEX_BITSET_MATCH_ANY) = 0
```

* **解析**：
  * 进程 `B` 调用 `futex` 进入等待状态，等待进程 `A` 的通知。

```c
newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=4096, ...}, AT_EMPTY_PATH) = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x725786dd1000
```

* **解析**：
  * 重新映射共享内存，确保数据的可用性。

```c
memcpy(buf, header->data, header->data_size); 
write(1, "[B] Received: Hello\n", 20[B] Received: Hello
)   = 20
```

* **解析**：
  * 从共享内存中读取数据（如 `"Hello"`）。
  * 使用 `write` 打印接收到的数据。

#### **8. 回应进程A**

```c
futex(0x725786dd1000, FUTEX_WAKE_PRIVATE, 1) = 0
futex(0x725786dd2000, FUTEX_WAKE, 1)    = 1
```

* **解析**：
  * 使用 `futex` 通知进程 `A` 数据已处理（通过信号量 `SEM_DONE`）。

#### **9. 清理资源**

```c
munmap(0x725786dd1000, 0)               = -1 EINVAL (Invalid argument)
close(3)                                = 0
munmap(0x725786dd3000, 32)              = 0
munmap(0x725786dd2000, 32)              = 0
```

* **解析**：
  * 使用 `munmap` 解除共享内存和信号量的映射。
  * 使用 `close` 关闭共享内存文件描述符。
