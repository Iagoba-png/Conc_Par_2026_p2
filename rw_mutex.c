#include "rw_mutex.h"
#include <pthread.h>
#include <stdlib.h>

struct rw_mutex_t {
    pthread_mutex_t lock;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    int readers;           // número de lectores activos
    int writer;            // 1 si hay escritor activo
    int waiting_writers;   // escritores esperando
};

int rw_mutex_init(rw_mutex_t *m) {
    int ret;
    ret = pthread_mutex_init(&m->lock, NULL);
    if (ret) return ret;
    ret = pthread_cond_init(&m->read_cond, NULL);
    if (ret) { pthread_mutex_destroy(&m->lock); return ret; }
    ret = pthread_cond_init(&m->write_cond, NULL);
    if (ret) { pthread_cond_destroy(&m->read_cond); pthread_mutex_destroy(&m->lock); return ret; }
    m->readers = 0;
    m->writer = 0;
    m->waiting_writers = 0;
    return 0;
}

int rw_mutex_destroy(rw_mutex_t *m) {
    pthread_mutex_destroy(&m->lock);
    pthread_cond_destroy(&m->read_cond);
    pthread_cond_destroy(&m->write_cond);
    return 0;
}

int rw_mutex_readlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    while (m->writer || m->waiting_writers > 0) {
        pthread_cond_wait(&m->read_cond, &m->lock);
    }
    m->readers++;
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rw_mutex_writelock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    m->waiting_writers++;
    while (m->readers > 0 || m->writer) {
        pthread_cond_wait(&m->write_cond, &m->lock);
    }
    m->waiting_writers--;
    m->writer = 1;
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rw_mutex_readunlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    m->readers--;
    if (m->readers == 0 && m->waiting_writers > 0) {
        pthread_cond_signal(&m->write_cond);
    }
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rw_mutex_writeunlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    m->writer = 0;
    if (m->waiting_writers > 0) {
        pthread_cond_signal(&m->write_cond);
    } else {
        pthread_cond_broadcast(&m->read_cond);
    }
    pthread_mutex_unlock(&m->lock);
    return 0;
}