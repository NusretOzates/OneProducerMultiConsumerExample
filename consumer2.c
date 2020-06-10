#include "pc_protocol.h"
#include <ctype.h>
#include <syslog.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

void shiftArrayLeft(int arr[], int size) {

    for (int i = 0; i < size - 1; ++i) {
        arr[i] = arr[i + 1];
    }
}

int main(int argc, char *argv[]) {

    char *shmpath = argv[1];
    char *p;
    if (argc < 4) {
        fprintf(stderr, "Usage: %s /shm-path producer no. \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    long timeInterval = strtol(argv[3], &p, 10);
    long cons_no = strtol(argv[2], &p, 10);

    int fd = shm_open(shmpath, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        errExit("shm_open");
    }

    struct shmbuf *shmp = mmap(NULL, sizeof(struct shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    openlog("consumer", LOG_NOWAIT, LOG_USER);
    int countelements = 0;

    while (1) {
        pthread_mutex_lock(&shmp->mutex);
        syslog(LOG_INFO,"Consumer %ld get the lock!",cons_no);
        if (shmp->sizeArr < 2) {
            countelements = 0;
            printf("Array ist leer. Consumer %ld wartet... \n", cons_no);
            printf("Verbrauchtete Elemente fur Consumer %ld : %d", cons_no, countelements);
            pthread_cond_signal(&shmp->arraynotfullcond);
            while (shmp->sizeArr < 2){
                pthread_cond_wait(&shmp->arraynotemptycond, &shmp->mutex);
            }

        }
        printf("\n");
        int item = shmp->buffer[0];
        printf("Consumer %ld: Remove Item %d from %d \n", cons_no, item, out);
        syslog(LOG_INFO,"Consumer %ld: Ein Element, %d, wird aus dem Shared Memory index %d entfernt",cons_no,item,0);
        countelements++;
        shiftArrayLeft(shmp->buffer, shmp->sizeArr);
        shmp->sizeArr--;
        printf("Size of the array is %d\n",shmp->sizeArr);
        int signal = pthread_cond_signal(&shmp->arraynotfullcond);
        if (signal!=0){errExit("OPS! Signal not successfull");}
        pthread_mutex_unlock(&shmp->mutex);
        closelog();
        delay(timeInterval);
    }
    return 0;
}

#pragma clang diagnostic pop

