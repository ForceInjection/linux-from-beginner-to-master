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