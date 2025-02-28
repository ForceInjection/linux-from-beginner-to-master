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