#include <string.h>
#include <syslog.h>
#include "pc_protocol.h"


void controlArraySize(struct shmbuf *shmp, int *elementCounter);

void lockMutex(long prod_no, struct shmbuf *shmp);

int addItem(int item, long prod_no, struct shmbuf *shmp, int elemenentCounter);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main(int argc, char *argv[]) {
    // There should be 4 argument to run the program
    if (argc != 4) {
        fprintf(stderr, "Usage: %s /shm-path producer no. Zeitintervall \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Path to shared memory
    char *shmpath = argv[1];
    char *p;
    int item;
    //Producer no
    long prod_no = strtol(argv[2], &p, 10);
    //time interval in ms
    long timeInterval = strtol(argv[3], &p, 10);

    //In case of there is a shared memory path, unlink it, otherwise we need to delete the folder everytime
    shm_unlink(shmpath);

    //Open or create the shared memory
    int fd = shm_open(shmpath, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        errExit("shm_open");
    }
    //Truncate its size by sizeof shmbuf struct
    if (ftruncate(fd, sizeof(struct shmbuf)) == -1) {
        errExit("truncate");
    }

    //We are mapping the memory area to our struct
    struct shmbuf *shmp = mmap(NULL, sizeof(struct shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    //Initialize mutex with process shared attiribute
    pthread_mutexattr_init(&shmp->mutexattr);
    pthread_mutexattr_setpshared(&shmp->mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shmp->mutex, &shmp->mutexattr);

    //Initialize condition variable with process shared attiribute
    pthread_condattr_init(&shmp->attr);
    pthread_condattr_setpshared(&shmp->attr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_init(&shmp->attr2);
    pthread_condattr_setpshared(&shmp->attr2, PTHREAD_PROCESS_SHARED);

    int x = pthread_cond_init(&shmp->arraynotemptycond, &shmp->attr);
    int y = pthread_cond_init(&shmp->arraynotfullcond, &shmp->attr2);

    if (x != 0 || y != 0) {
        errExit("condINIT");
    }


    //Add all items to the buffer for the first time
    pthread_mutex_lock(&shmp->mutex);

    shmp->sizeArr = BufferSize;
    openlog("producer", LOG_NOWAIT, LOG_USER);
    for (int i = 0; i < MaxItems; ++i) {
        item = i * 10;
        shmp->buffer[in] = item;
        printf("Producer %ld: Ein element %d, wird in das Shared Memory geschrieben\n", prod_no, shmp->buffer[in]);
        syslog(LOG_INFO, "Producer %ld: Ein Element, %d, wird aus dem Shared Memory index %d geschrieben.", prod_no,
               item, in);
        in = (in + 1) % BufferSize;
    }

    pthread_cond_broadcast(&shmp->arraynotemptycond);
    pthread_mutex_unlock(&shmp->mutex);

    delay(2000L);

    int elemenentCounter = 0;
    in = 0;
    while (1) {
        //Lock
        lockMutex(prod_no, shmp);
        //Wait if buffer is full
        controlArraySize(shmp, &elemenentCounter);
        //Add item and increment elementCounter
        elemenentCounter = addItem(item, prod_no, shmp, elemenentCounter);

        //Unlock
        pthread_cond_broadcast(&shmp->arraynotemptycond);
        pthread_mutex_unlock(&shmp->mutex);
        delay(timeInterval);
    }
    return 0;
}

int addItem(int item, long prod_no, struct shmbuf *shmp, int elemenentCounter) {
    //Shortcut pointer to array size
    int *size = &shmp->sizeArr;
    item = *size * 10;
    //Add to end of the array
    shmp->buffer[*size] = item;
    printf("Producer %ld: Ein element %d, wird in das Shared Memory geschrieben index : %d \n", prod_no,
           item, *size);
    syslog(LOG_INFO, "Producer %ld: Ein Element, %d, wird aus dem Shared Memory index %d geschrieben.", prod_no,
           item, *size);
    //Increment item size and elementcounter
    // Elementcounter = how many time producer produce an item
    *size += 1;
    elemenentCounter++;
    return elemenentCounter;
}

void lockMutex(long prod_no, struct shmbuf *shmp) {
    printf("Mutex bekleniyor\n");
    pthread_mutex_lock(&shmp->mutex);
    syslog(LOG_INFO, "Producer %ld get the lock!", prod_no);
}

void controlArraySize(struct shmbuf *shmp, int *elementCounter) {
    if (shmp->sizeArr == MaxItems) {
        printf("Array ist voll!\n");
        printf("produzierte Elemente fur Producer %d\n", *elementCounter);
        *elementCounter = 0;
        while (shmp->sizeArr >= MaxItems) {
            pthread_cond_wait(&shmp->arraynotfullcond, &shmp->mutex);
        }
    }
}

#pragma clang diagnostic pop