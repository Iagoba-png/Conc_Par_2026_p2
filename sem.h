#ifndef __SEM_H__
#define __SEM_H__

typedef struct sem_t sem_t;

int sem_init(sem_t *s, int value);
int sem_destroy(sem_t *s);

int sem_p(sem_t *s);
int sem_v(sem_t *s);
int sem_tryp(sem_t *s); // 0 on sucess, -1 if already locked

#endif