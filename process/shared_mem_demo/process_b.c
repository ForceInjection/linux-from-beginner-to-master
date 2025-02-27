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
