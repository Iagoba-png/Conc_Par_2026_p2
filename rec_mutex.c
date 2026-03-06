#include "rec_mutex.h"
#include <pthread.h>
#include <stdlib.h>

struct rec_mutex_t {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t owner;
    int count;
};

int rec_mutex_init(rec_mutex_t *m) {
    int ret;
    ret = pthread_mutex_init(&m->lock, NULL);
    if (ret != 0) return ret;
    ret = pthread_cond_init(&m->cond, NULL);
    if (ret != 0) {
        pthread_mutex_destroy(&m->lock);
        return ret;
    }
    m->owner = 0;
    m->count = 0;
    return 0;
}

int rec_mutex_destroy(rec_mutex_t *m) {
    pthread_mutex_destroy(&m->lock);
    pthread_cond_destroy(&m->cond);
    return 0;
}

int rec_mutex_lock(rec_mutex_t *m) {
    pthread_t self = pthread_self();
    pthread_mutex_lock(&m->lock);
    if (pthread_equal(self, m->owner)) {

        m->count++;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }

    while (m->count > 0) {
        pthread_cond_wait(&m->cond, &m->lock);
    }

    m->owner = self;
    m->count = 1;
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rec_mutex_unlock(rec_mutex_t *m) {
    pthread_t self = pthread_self();
    pthread_mutex_lock(&m->lock);
    if (!pthread_equal(self, m->owner)) {
        pthread_mutex_unlock(&m->lock);
        return -1;
    }
    m->count--;
    if (m->count == 0) {
        m->owner = 0;
        pthread_cond_signal(&m->cond);
    }
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rec_mutex_trylock(rec_mutex_t *m) {
    pthread_t self = pthread_self();
    pthread_mutex_lock(&m->lock);
    if (pthread_equal(self, m->owner)) {

        m->count++;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }
    if (m->count == 0) {

        m->owner = self;
        m->count = 1;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }

    pthread_mutex_unlock(&m->lock);
    return -1;
}
