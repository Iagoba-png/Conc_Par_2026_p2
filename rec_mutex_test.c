#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "rec_mutex.c"

#define ARRAY_SIZE 10
#define NUM_THREADS 4
#define ITERATIONS 100

typedef struct {
    int array[ARRAY_SIZE];
    rec_mutex_t *mutex;
}data;


void* worker(void* args) {
    data *datos = (data*)args;
    unsigned int seed = (unsigned int)pthread_self();
    for (int i = 0; i < ITERATIONS; i++) {
        int idx1 = rand() % ARRAY_SIZE;
        int idx2 = rand() % ARRAY_SIZE;
        if (idx1 == idx2) continue;

        rec_mutex_lock(&datos->mutex);
        int temp = datos->array[idx1];
        datos->array[idx1] = datos->array[idx2];
        datos->array[idx2] = temp;

        rec_mutex_unlock(&datos->mutex);
    }
    return NULL;
}

int main() {
    data data;

    rec_mutex_init(&data.mutex);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        data.array[i] = i;
    }

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, &data);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += data.array[i];
        printf("%d ", data.array[i]);
    }
    printf("\nSuma: %d (debe ser %d)\n", sum, ARRAY_SIZE*(ARRAY_SIZE-1)/2);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        rec_mutex_destroy(data.mutex);
    }

    return 0;
}