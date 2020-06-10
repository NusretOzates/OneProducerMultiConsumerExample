#include "pc_protocol.h"
#include <ctype.h>
#include <syslog.h>

int controlArraySizeandWait(long cons_no, const struct shmbuf *shmp, int countelements);

int consume(long cons_no, struct shmbuf *shmp, int countelements);

void shiftArrayLeft(int arr[], int size);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"


int main(int argc, char *argv[]) {
    // There should be 4 argument to run the program
    if (argc < 4) {
        fprintf(stderr, "Usage: %s /shm-path producer no. \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //Path to shared memory
    char *shmpath = argv[1];
    char *p;

    //time interval in ms
    long timeInterval = strtol(argv[3], &p, 10);
    //Producer no
    long cons_no = strtol(argv[2], &p, 10);
    //Open the shared memory
    int fd = shm_open(shmpath, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        errExit("shm_open");
    }

    //We are mapping the memory area to our struct
    struct shmbuf *shmp = mmap(NULL, sizeof(struct shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    openlog("consumer", LOG_NOWAIT, LOG_USER);
    int countelements = 0;

    while (1) {
        pthread_mutex_lock(&shmp->mutex);

        syslog(LOG_INFO, "Consumer %ld get the lock!", cons_no);
        //Wait if Necessary
        countelements = controlArraySizeandWait(cons_no, shmp, countelements);

        printf("\n");
        //Consume 1 element from 0.th index and increment counter
        countelements = consume(cons_no, shmp, countelements);

        printf("Size of the array is %d\n", shmp->sizeArr);

        int signal = pthread_cond_signal(&shmp->arraynotfullcond);
        if (signal != 0) { errExit("OPS! Signal not successfull"); }
        pthread_mutex_unlock(&shmp->mutex);
        closelog();
        delay(timeInterval);
    }
    return 0;
}

int consume(long cons_no, struct shmbuf *shmp, int countelements) {
    int item = shmp->buffer[0];

    printf("Consumer %ld: Remove Item %d from %d \n", cons_no, item, out);
    syslog(LOG_INFO, "Consumer %ld: Ein Element, %d, wird aus dem Shared Memory index %d entfernt", cons_no, item, 0);

    countelements++;
    shiftArrayLeft(shmp->buffer, shmp->sizeArr);
    shmp->sizeArr--;
    return countelements;
}

int controlArraySizeandWait(long cons_no, const struct shmbuf *shmp, int countelements) {
    if (shmp->sizeArr < 2) {
        countelements = 0;

        printf("Array ist leer. Consumer %ld wartet... \n", cons_no);
        printf("Verbrauchtete Elemente fur Consumer %ld : %d", cons_no, countelements);

        pthread_cond_signal(&shmp->arraynotfullcond);

        while (shmp->sizeArr < 2) {
            pthread_cond_wait(&shmp->arraynotemptycond, &shmp->mutex);
        }
    }
    return countelements;
}

void shiftArrayLeft(int arr[], int size) {

    for (int i = 0; i < size - 1; ++i) {
        arr[i] = arr[i + 1];
    }
}

#pragma clang diagnostic pop

