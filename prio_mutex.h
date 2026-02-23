#ifndef __PRIO_MUTEX_H__
#define __PRIO_MUTEX_H__


typedef struct prio_mutex_t prio_mutex_t;

int prio_mutex_init(prio_mutex_t *m, int num_prios);
int prio_mutex_destroy(prio_mutex_t *m);

int prio_mutex_lock(prio_mutex_t *m, int prio);
int prio_mutex_unlock(prio_mutex_t *m);
int prio_mutex_trylock(prio_mutex_t *m); // 0 if sucessful, -1 if already locked

#endif