#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define NANO_SECOND_MULTIPLIER  1000000  // 1 millisecond = 1,000,000 Nanoseconds

#define errExit(msg) do{perror(msg) ; exit(EXIT_FAILURE);}while(0)

#define BufferSize 100
#define MaxItems 100

int in = 0;
int out = 0;

struct shmbuf {

    sem_t full;
    pthread_cond_t arraynotfullcond;
    pthread_cond_t arraynotemptycond;
    pthread_condattr_t attr;
    pthread_condattr_t attr2;
    int sizeArr;
    int buffer[BufferSize];
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexattr;
};

int delay(unsigned int ms) {
    int result = 0;

    {
        struct timespec ts_remaining =
                {
                        ms / 1000,
                        (ms % 1000) * 1000000L
                };

        do {
            struct timespec ts_sleep = ts_remaining;
            result = nanosleep(&ts_sleep, &ts_remaining);
        } while ((EINTR == errno) && (-1 == result));
    }

    return result;
}
