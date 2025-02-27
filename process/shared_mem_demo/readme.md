# Linux 进程间共享内存通信方案

## 一、共享内存通信原理

共享内存是一种高效的进程间通信机制，允许多个进程直接访问同一块内存区域，从而实现数据的快速交换。以下是共享内存通信的基本原理：

### 1. 内存共享

- **内存映射**：通过系统调用（如 `mmap`），进程将共享内存对象映射到自己的地址空间。这样，各进程如同操作本地内存一样读取和写入共享数据，避免了数据拷贝的开销。
- **同步问题**：由于多个进程可以同时访问共享内存，必须使用同步机制（如互斥锁、信号量）来防止数据竞争和不一致。

### 2. 信号量机制

- **生产者-消费者模型**：进程A（生产者）向共享内存写入数据，通过信号量通知进程B（消费者）有数据可读；进程B处理完数据后，用信号量告知进程A可以继续写入。
- **实例化信号量**：初始化的信号量 `/sem_avail` 和 `/sem_done` 用于协调生产者和消费者的交互，确保数据的有序读写。

---

## 二、设计目标

1. 进程 `A` 负责读写共享内存
2. 进程 `B` 只读取共享内存
3. 进程 `A` 需要通知进程B读取数据
4. 进程 `B` 需要通知进程A数据已读取完毕
5. 支持运行时动态调整共享内存大小

---

## 三、整体架构

```plaintext
+----------------+          POSIX          +----------------+
|   进程A        |  <---- 命名信号量 ------> |       进程B     |
|   (读写)       |                         |       (只读)    |
+----------------+                         +----------------+
        |                                       |
        |         共享内存区域                 |
        +-------> [Header][Data...] <-----------+
                   │  │  │ 
                   │  │  └── 实际数据存储区
                   │  └─── 互斥锁+元数据
                   └──── 内存头部信息
```

---

## 四、关键技术实现

### 1. 共享内存设计

#### 内存头部结构

```c
struct shm_header {
    pthread_mutex_t mutex;      // 进程间互斥锁
    size_t capacity;            // 当前共享内存总容量
    size_t data_size;           // 有效数据长度
    volatile int ready_flag;    // 数据就绪标志
    char data[];                // 柔性数组-数据存储区
};
```

#### 动态调整流程

**（1）进程 A 调整大小**：

```plaintext
A. 加锁：pthread_mutex_lock(&header->mutex) → 防止其他进程修改数据结构；
B. 解除旧映射：munmap(header, header->capacity) → 删除当前内存映射；
C. 扩容：ftruncate(shm_fd, new_capacity) → 扩展共享内存对象大小；
D. 新映射：mmap(...) → 创建新的、更大的内存映射；
E. 更新元数据：header->capacity = new_capacity → 修改共享内存容量；
F. 解锁：pthread_mutex_unlock(&header->mutex) → 释放互斥锁。
```

**（2）进程 B 同步感知**：

```plaintext
A. 加锁：pthread_mutex_lock(&header->mutex) → 获取访问权限；
B. fstat检查文件大小：获取共享内存对象的最新状态；
C. 若大小变化，则重新映射：munmap(...) 和 mmap(...)；
D. 解锁：pthread_mutex_unlock(&header->mutex)。
```

### 2. 同步机制设计

#### 信号量配置

| **信号量名称**   | **初始值** | **用途说明**                  |
|--------------|--------|--------------------------|
| `/sem_avail` | 0      | 数据就绪通知（A→B）      |
| `/sem_done`  | 0      | 数据消费完成通知（B→A）  |

#### 互斥锁属性

```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);  // 设为进程间共享属性
pthread_mutex_init(&header->mutex, &attr);
```

---

## 五、具体实现步骤

### 1. common.h

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

#define SHM_NAME "/demo_shm"
#define SEM_AVAIL "/sem_avail"
#define SEM_DONE "/sem_done"
#define INIT_SIZE 4096

struct shm_header {
    pthread_mutex_t mutex;
    size_t capacity;
    size_t data_size;
    volatile int ready_flag;
    char data[];
};
```

### 2. process_a.c

```c
#include "common.h"

int main() {
    // 创建共享内存
    // 使用 shm_open 创建一个共享内存对象，O_CREAT 表示创建，O_RDWR 表示读写权限
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");  // 如果共享内存创建失败，打印错误并退出
        exit(EXIT_FAILURE);
    }

    // 设置共享内存的大小
    // 使用 ftruncate 设置共享内存对象的大小为 INIT_SIZE
    if (ftruncate(shm_fd, INIT_SIZE) == -1) {
        perror("ftruncate failed");  // 如果设置共享内存大小失败，打印错误并退出
        exit(EXIT_FAILURE);
    }

    // 内存映射
    // 使用 mmap 将共享内存映射到当前进程的虚拟地址空间
    struct shm_header *header = mmap(NULL, INIT_SIZE, 
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shm_fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap failed");  // 如果内存映射失败，打印错误并退出
        exit(EXIT_FAILURE);
    }

    // 初始化互斥锁
    // 为了确保共享内存在多个进程之间访问时不会产生竞态条件，需要用互斥锁进行同步
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);  // 初始化互斥锁属性
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);  // 设置互斥锁在进程间共享
    if (pthread_mutex_init(&header->mutex, &attr) != 0) {
        perror("pthread_mutex_init failed");  // 如果互斥锁初始化失败，打印错误并退出
        exit(EXIT_FAILURE);
    }

    // 初始化共享内存元数据
    header->capacity = INIT_SIZE;  // 设置共享内存的初始容量
    header->data_size = 0;         // 设置数据大小为 0
    header->ready_flag = 0;        // 设置数据就绪标志为 0，表示没有数据准备好

    // 创建信号量
    // 信号量用于在进程之间进行同步
    sem_unlink(SEM_AVAIL);  // 删除已存在的信号量（如果有的话）
    sem_unlink(SEM_DONE);   // 删除已存在的信号量（如果有的话）

    // 创建新的信号量，信号量初始值为 0
    sem_t *sem_avail = sem_open(SEM_AVAIL, O_CREAT | O_EXCL, 0666, 0);  
    if (sem_avail == SEM_FAILED) {
        perror("sem_open failed for SEM_AVAIL");  // 如果信号量创建失败，打印错误并退出
        exit(EXIT_FAILURE);
    }

    sem_t *sem_done = sem_open(SEM_DONE, O_CREAT | O_EXCL, 0666, 0);  
    if (sem_done == SEM_FAILED) {
        perror("sem_open failed for SEM_DONE");  // 如果信号量创建失败，打印错误并退出
        exit(EXIT_FAILURE);
    }

    // 示例数据
    // 定义一个包含多个消息的字符串数组，这些消息将被写入共享内存
    const char *messages[] = {"Hello", "World", "Dynamic", "Memory"};
    
    // 循环发送数据给进程B
    for (int i = 0; i < 4; i++) {
        size_t data_len = strlen(messages[i]) + 1;  // 计算当前消息的长度（包含 '\0' 结束符）

        // 获取互斥锁，确保共享内存访问的安全
        pthread_mutex_lock(&header->mutex);

        // 检查共享内存容量是否足够存储新数据
        if (data_len > (header->capacity - sizeof(struct shm_header))) {
            // 如果当前容量不足，扩展共享内存的容量
            size_t new_cap = header->capacity * 2;  // 将容量翻倍，作为新的容量大小
            while (new_cap < (sizeof(struct shm_header) + data_len)) {
                new_cap *= 2;  // 如果新的容量仍然不足，继续翻倍
            }

            // 解除当前内存映射，释放已分配的内存
            munmap(header, header->capacity);

            // 调整共享内存大小
            if (ftruncate(shm_fd, new_cap) == -1) {
                perror("ftruncate failed during resizing");  // 如果调整大小失败，打印错误并退出
                exit(EXIT_FAILURE);
            }

            // 重新映射共享内存，以适应新容量
            header = mmap(NULL, new_cap, 
                          PROT_READ | PROT_WRITE, 
                          MAP_SHARED, shm_fd, 0);
            if (header == MAP_FAILED) {
                perror("mmap failed after resizing");  // 如果映射失败，打印错误并退出
                exit(EXIT_FAILURE);
            }

            header->capacity = new_cap;  // 更新共享内存的容量
        }

        // 写入数据到共享内存
        memcpy(header->data, messages[i], data_len);  // 将消息复制到共享内存
        header->data_size = data_len;  // 更新数据的大小
        header->ready_flag = 1;  // 设置数据已准备好的标志

        // 释放互斥锁
        pthread_mutex_unlock(&header->mutex);

        // 打印发送的数据
        printf("[A] Sent: %s\n", messages[i]);

        // 通知进程B数据已经准备好
        sem_post(sem_avail);

        // 等待进程B确认数据已经被读取
        sem_wait(sem_done);
    }

    // 清理资源
    // 解除内存映射，关闭文件描述符和信号量
    munmap(header, header->capacity);
    close(shm_fd);
    
    sem_close(sem_avail);
    sem_close(sem_done);

    // 删除信号量
    sem_unlink(SEM_AVAIL);
    sem_unlink(SEM_DONE);

    return 0;
}
```

### 3. process_b.c

```c
#include "common.h"

int main() {
    // 打开共享内存
    // 使用 shm_open 以只读写模式打开已创建的共享内存对象
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");  // 打开共享内存失败时打印错误信息
        exit(EXIT_FAILURE);
    }

    // 获取当前共享内存的大小
    // 使用 fstat 获取共享内存的元数据（比如大小）
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("fstat failed");  // 如果 fstat 失败，打印错误信息并退出
        exit(EXIT_FAILURE);
    }

    // 内存映射
    // 使用 mmap 将共享内存映射到进程的地址空间，以便进行读写操作
    struct shm_header *header = mmap(NULL, shm_stat.st_size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shm_fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap failed");  // 如果 mmap 失败，打印错误信息并退出
        exit(EXIT_FAILURE);
    }

    // 打开信号量
    // 打开由进程A创建的信号量，以便进行同步操作
    sem_t *sem_avail = sem_open(SEM_AVAIL, 0);
    if (sem_avail == SEM_FAILED) {
        perror("sem_open failed for SEM_AVAIL");  // 打开信号量失败，打印错误信息并退出
        exit(EXIT_FAILURE);
    }

    sem_t *sem_done = sem_open(SEM_DONE, 0);
    if (sem_done == SEM_FAILED) {
        perror("sem_open failed for SEM_DONE");  // 打开信号量失败，打印错误信息并退出
        exit(EXIT_FAILURE);
    }

    // 循环接收来自进程A的数据
    for (int i = 0; i < 4; i++) {
        // 等待信号量，直到进程A有数据可供读取
        sem_wait(sem_avail);

        // 获取互斥锁以确保线程安全地操作共享内存
        pthread_mutex_lock(&header->mutex);

        // 检查共享内存的大小是否发生变化
        // 如果共享内存的大小比当前映射的内存容量更大，重新映射
        if (fstat(shm_fd, &shm_stat) == -1) {
            perror("fstat failed");  // 如果 fstat 失败，打印错误信息并退出
            exit(EXIT_FAILURE);
        }

        if (shm_stat.st_size > header->capacity) {
            // 如果共享内存增长，解除当前内存映射并重新映射更大的内存
            munmap(header, header->capacity);  // 解除当前内存映射

            // 重新映射共享内存区域以适应新大小
            header = mmap(NULL, shm_stat.st_size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd, 0);
            if (header == MAP_FAILED) {
                perror("mmap failed after resizing");  // 如果 mmap 失败，打印错误信息并退出
                exit(EXIT_FAILURE);
            }
        }

        // 读取数据
        if (header->ready_flag) {
            char buf[256];  // 临时缓冲区存放读取的数据
            memcpy(buf, header->data, header->data_size);  // 从共享内存中复制数据到缓冲区
            header->ready_flag = 0;  // 重置 ready_flag，表示数据已被进程B读取
            printf("[B] Received: %s\n", buf);  // 输出接收到的数据
        }

        // 解锁互斥锁，允许其他进程访问共享内存
        pthread_mutex_unlock(&header->mutex);

        // 通知进程A数据已被处理
        sem_post(sem_done);
    }

    // 清理资源
    // 解除内存映射并关闭共享内存文件描述符
    munmap(header, shm_stat.st_size);
    close(shm_fd);

    // 关闭信号量
    sem_close(sem_avail);
    sem_close(sem_done);

    return 0;
}
```

### 4. Makefile

```makefile
CC = gcc
CFLAGS = -Wall -O2 -pthread

all: process_a process_b

process_a: process_a.c common.h
 $(CC) $(CFLAGS) $< -o $@

process_b: process_b.c common.h
 $(CC) $(CFLAGS) $< -o $@

clean:
 rm -f process_a process_b
 rm -f /dev/shm/*
```

**build**：

```bash
make clean && make
```

### 5. Run

**terminal #1**：

```bash
./process_a
[A] Sent: Hello
[A] Sent: World
[A] Sent: Dynamic
[A] Sent: Memory
```

**terminal #2**：

```bash
./process_b
[B] Received: Hello
[B] Received: World
[B] Received: Dynamic
[B] Received: Memory
```

## 六、关键系统调用说明

### （一）`shm_open`

| **参数** | **说明** |
|----------|----------|
| `name`   | 共享内存对象的名字，通常以 `/` 开头，如 `/my_shm` |
| `flags`  | 指定操作模式，如 `O_CREAT`（若对象不存在则创建）、`O_RDWR`（读写权限）等 |
| `mode`   | 设置对象的权限位（类似文件权限），如 `0666` 表示读写权限 |
| **返回值** | 成功返回一个文件描述符；出错返回 `-1`，并设置 `errno` |

### （二）`ftruncate`

| **参数** | **说明** |
|----------|----------|
| `fd`     | 由 `shm_open` 等调用返回的文件描述符 |
| `length` | 要设置的新大小（以字节为单位） |
| **返回值** | 成功返回 `0`；出错返回 `-1` |

### （三）`mmap`

| **参数** | **说明** |
|----------|----------|
| `addr`   | 请求映射的起始地址（通常设为 `NULL`，由系统选择） |
| `length` | 映射区域的大小 |
| `prot`   | 指定内存保护标志，如 `PROT_READ`（可读）、`PROT_WRITE`（可写） |
| `flags`  | 指定映射的附加属性，如 `MAP_SHARED`（写操作同步到内存对象）、`MAP_PRIVATE`（写操作不共享） |
| `fd`     | 文件描述符 |
| `offset` | 映射的起始偏移量（通常为 `0`） |
| **返回值** | 成功返回映射的地址；出错返回 `(void *) -1` |

### （四）`sem_open`

| **参数** | **说明** |
|----------|----------|
| `name`   | 信号量的名字（如 `/sem_avail`） |
| `oflag`  | 创建标志（如 `O_CREAT`，表示若信号量不存在则创建） |
| `mode`   | 权限位（可忽略，通常设为 `0666`） |
| `value`  | 若创建新信号量，则指定初始值 |
| **返回值** | 成功返回信号量指针；出错返回 `SEM_FAILED` |

### （五）`pthread_mutexattr_init`

| **参数** | **说明** |
|----------|----------|
| `attr`   | 互斥锁属性对象的指针 |
| **返回值** | 成功返回 `0`；出错返回错误码 |

### （六）`pthread_mutexattr_setpshared`

| **参数** | **说明** |
|----------|----------|
| `attr`   | 互斥锁属性对象的指针 |
| `pshared` | 共享属性值，如 `PTHREAD_PROCESS_SHARED`（进程间共享） |
| **返回值** | 成功返回 `0`；出错返回错误码 |

### （七）`pthread_mutex_init`

| **参数** | **说明** |
|----------|----------|
| `mutex`  | 互斥锁的指针 |
| `attr`   | 互斥锁属性对象的指针（可为 `NULL`） |
| **返回值** | 成功返回 `0`；出错返回错误码 |

## 七、方案优缺点分析

### 1. 优点

1. **高效通信**：共享内存直接访问，避免数据拷贝
2. **动态扩容**：按需调整内存大小，支持大数据传输
3. **强同步机制**：双重保障（信号量+互斥锁）
4. **数据完整性**：通过就绪标志防止脏读

### 2. 缺点

1. **实现复杂度高**：需要处理多种同步机制
2. **平台依赖性**：`POSIX`特性在 `Windows` 兼容性差
3. **调试难度大**：共享内存问题难以跟踪

---

## 八、扩展建议

1. **多进程支持**：可扩展为 **1** 对多通信模型
2. **心跳检测**：增加健康检查机制
3. **日志系统**：记录内存操作日志
4. **性能监控**：统计内存使用率和吞吐量

## 九、参考

- [深入理解Linux内核共享内存机制- shmem&tmpfs](https://mp.weixin.qq.com/s/mc5PgpAn6PLhmsytZbdvsw)
