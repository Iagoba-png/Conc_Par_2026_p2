#include "sem.h"
#include <pthread.h>
#include <stdlib.h>

int sem_init(sem_t *s, int value) {
    if (s == NULL) return -1;
    if (pthread_mutex_init(&s->mutex, NULL)!=0) {
        return -1;
    }
    if (pthread_cond_init(&s->cond, NULL) != 0) {
        pthread_mutex_destroy(&s->mutex);
        return -1;
    }
    s->value = value;
    return 0;
}

int sem_destroy(sem_t *s) {
    if (s == NULL) return -1;
    pthread_mutex_destroy(&s->mutex);
    pthread_cond_destroy(&s->cond);
    return 0;
}

int sem_p(sem_t *s) {
    pthread_mutex_lock(&s->mutex);
    while (s->value == 0) {
        pthread_cond_wait(&s->cond, &s->mutex);
    }
    s->value--;
    pthread_mutex_unlock(&s->mutex);
    return 0;
}

int sem_v(sem_t *s) {
    pthread_mutex_lock(&s->mutex);
    s->value++;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
    return 0;
}

int sem_tryp(sem_t *s) {
    pthread_mutex_lock(&s->mutex);
    if (s->value > 0) {
        s->value--;
        pthread_mutex_unlock(&s->mutex);
        return 0;
    } else {
        pthread_mutex_unlock(&s->mutex);
        return -1;
    }
}
