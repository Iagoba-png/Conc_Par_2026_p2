#include "rec_mutex.h"
#include <pthread.h>
#include <stdlib.h>

struct rec_mutex_t {
    pthread_mutex_t lock;       // protege la estructura
    pthread_cond_t cond;        // para esperar si está ocupado
    pthread_t owner;            // hilo que posee el mutex (0 si ninguno)
    int count;                  // contador de anidamiento
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
        // Ya tenemos el mutex, solo incrementamos contador
        m->count++;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }
    // Esperamos hasta que esté libre
    while (m->count > 0) {
        pthread_cond_wait(&m->cond, &m->lock);
    }
    // Ahora somos el dueño
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
        return -1;   // error: no es el dueño
    }
    m->count--;
    if (m->count == 0) {
        m->owner = 0;
        pthread_cond_signal(&m->cond);   // despierta a un posible waiter
    }
    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rec_mutex_trylock(rec_mutex_t *m) {
    pthread_t self = pthread_self();
    pthread_mutex_lock(&m->lock);
    if (pthread_equal(self, m->owner)) {
        // Ya lo tenemos, podemos anidar
        m->count++;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }
    if (m->count == 0) {
        // Libre, lo tomamos
        m->owner = self;
        m->count = 1;
        pthread_mutex_unlock(&m->lock);
        return 0;
    }
    // Ocupado por otro hilo
    pthread_mutex_unlock(&m->lock);
    return -1;
}
