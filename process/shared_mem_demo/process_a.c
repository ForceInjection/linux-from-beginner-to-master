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
