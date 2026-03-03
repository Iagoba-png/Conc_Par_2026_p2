#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "rw_mutex.c"

#define NUM_READERS 3
#define NUM_WRITERS 2
#define ITER 5

rw_mutex_t rw;
int shared_data = 0;

void* reader(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < ITER; i++) {
        rw_mutex_readlock(&rw);
        printf("Lector %d lee: %d\n", id, shared_data);
        usleep(100000);
        rw_mutex_readunlock(&rw);
        usleep(50000);
    }
    return NULL;
}

void* writer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < ITER; i++) {
        rw_mutex_writelock(&rw);
        shared_data++;
        printf("Escritor %d escribe: %d\n", id, shared_data);
        usleep(150000);
        rw_mutex_writeunlock(&rw);
        usleep(50000);
    }
    return NULL;
}

int main() {
    rw_mutex_init(&rw);

    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int ids_reader[NUM_READERS], ids_writer[NUM_WRITERS];

    for (int i = 0; i < NUM_READERS; i++) {
        ids_reader[i] = i+1;
        pthread_create(&readers[i], NULL, reader, &ids_reader[i]);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        ids_writer[i] = i+1;
        pthread_create(&writers[i], NULL, writer, &ids_writer[i]);
    }

    for (int i = 0; i < NUM_READERS; i++) pthread_join(readers[i], NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_join(writers[i], NULL);

    rw_mutex_destroy(&rw);
    return 0;
}
