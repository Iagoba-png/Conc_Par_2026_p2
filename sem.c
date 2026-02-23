struct sem_t {
};

#include "sem.h"

int sem_init(sem_t *s, int value) {
    return 0;
}

int sem_destroy(sem_t *s) {
    return 0;
}

int sem_p(sem_t *s) {
    return 0;
}

int sem_v(sem_t *s) {
    return 0;
}

int sem_tryp(sem_t *s) { // 0 on sucess, -1 if already locked
    return 0;
}
