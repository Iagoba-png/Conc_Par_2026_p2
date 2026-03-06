#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "prio_mutex.c"

#define NUM_PRIOS 3


void* thread_high(void* arg) {
    prio_mutex_t *m = (prio_mutex_t *) arg;
    printf("Hilo alta (prio 2) intenta bloquear...\n");
    prio_mutex_lock(m, 2);
    printf("Alta adquiere, trabaja...\n");
    printf("Alta libera\n");
    prio_mutex_unlock(m);
    return NULL;
}

void* thread_medium(void* arg) {
    prio_mutex_t *m = (prio_mutex_t *) arg;
    printf("Hilo media (prio 1) intenta bloquear...\n");
    prio_mutex_lock(m, 1);
    printf("Media adquiere, trabaja...\n");
    printf("Media libera\n");
    prio_mutex_unlock(m);
    return NULL;
}

void* thread_low(void* arg) {
    prio_mutex_t *m = (prio_mutex_t *) arg;
    printf("Hilo baja (prio 0) intenta bloquear...\n");
    prio_mutex_lock(m, 0);
    printf("Baja adquiere, trabaja...\n");
    printf("Baja libera\n");
    prio_mutex_unlock(m);
    return NULL;
}

int main() {
    prio_mutex_t mutex;
    prio_mutex_init(&mutex, NUM_PRIOS);

    printf("Main bloquea con prioridad baja\n");
    prio_mutex_lock(&mutex, 0);

    pthread_t t_high, t_med, t_low;
    pthread_create(&t_low, NULL, thread_low, &mutex);
    pthread_create(&t_high, NULL, thread_high, &mutex);
    pthread_create(&t_med, NULL, thread_medium, &mutex);

    usleep(50000);
    printf("Main libera, ahora debe adquirir el de mayor prioridad\n");
    prio_mutex_unlock(&mutex);

    pthread_join(t_high, NULL);
    pthread_join(t_med, NULL);
    pthread_join(t_low, NULL);

    prio_mutex_destroy(&mutex);
    return 0;
}