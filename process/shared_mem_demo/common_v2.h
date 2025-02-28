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