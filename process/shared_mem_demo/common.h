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
