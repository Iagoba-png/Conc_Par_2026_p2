#include "prio_mutex.h"
#include <pthread.h>
#include <stdlib.h>

struct prio_mutex_t {
    pthread_mutex_t lock;
    int locked;
    int num_prios;
    int *waiting_counts;
    pthread_cond_t *conds;
};

int prio_mutex_init(prio_mutex_t *m, int num_prios) {
    int ret;
    ret = pthread_mutex_init(&m->lock, NULL);
    if (ret) return ret;

    m->locked = 0;
    m->num_prios = num_prios;
    m->waiting_counts = calloc(num_prios, sizeof(int));
    if (!m->waiting_counts) {
        pthread_mutex_destroy(&m->lock);
        return -1;
    }
    m->conds = malloc(num_prios * sizeof(pthread_cond_t));
    if (!m->conds) {
        free(m->waiting_counts);
        pthread_mutex_destroy(&m->lock);
        return -1;
    }
    for (int i = 0; i < num_prios; i++) {
        ret = pthread_cond_init(&m->conds[i], NULL);
        if (ret) {
            for (int j = 0; j < i; j++) pthread_cond_destroy(&m->conds[j]);
            free(m->conds);
            free(m->waiting_counts);
            pthread_mutex_destroy(&m->lock);
            return ret;
        }
    }
    return 0;
}

int prio_mutex_destroy(prio_mutex_t *m) {
    pthread_mutex_destroy(&m->lock);
    for (int i = 0; i < m->num_prios; i++)
        pthread_cond_destroy(&m->conds[i]);
    free(m->conds);
    free(m->waiting_counts);
    return 0;
}

static int has_higher_waiting(prio_mutex_t *m, int prio) {
    for (int p = m->num_prios - 1; p > prio; p--)
        if (m->waiting_counts[p] > 0) return 1;
    return 0;
}

int prio_mutex_lock(prio_mutex_t *m, int prio) {
    if (prio < 0 || prio >= m->num_prios) return -1;
    pthread_mutex_lock(&m->lock);
    while (m->locked || has_higher_waiting(m, prio)) {
        m->waiting_counts[prio]++;
        pthread_cond_wait(&m->conds[prio], &m->lock);
        m->waiting_counts[prio]--;
    }
    m->locked = 1;
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int prio_mutex_unlock(prio_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    m->locked = 0;

    for (int p = m->num_prios - 1; p >= 0; p--) {
        if (m->waiting_counts[p] > 0) {
            pthread_cond_signal(&m->conds[p]);
            break;
        }
    }
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int prio_mutex_trylock(prio_mutex_t *m) {
    pthread_mutex_lock(&m->lock);
    if (!m->locked) {
        m->locked = 1;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }
    pthread_mutex_unlock(&m->lock);
    return -1;
}