#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "options.h"
#include "sem.h"

#define CHAIRS 3

sem_t customers;
sem_t barbers;
sem_t free_seats;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

int cut_time;

void* barber_thread(void* arg) {
    int id = *(int*)arg;
    while (1) {
        sem_p(&customers);
        sem_v(&barbers);
        pthread_mutex_lock(&print_mutex);
        printf("Barbero %d corta el pelo\n", id);
        pthread_mutex_unlock(&print_mutex);
        usleep(cut_time);
    }
    return NULL;
}

void* customer_thread(void* arg) {
    int id = *(int*)arg;

    usleep(rand() % 1000000);

    if (sem_tryp(&free_seats) == 0) {
        pthread_mutex_lock(&print_mutex);
        printf("Cliente %d entra a la sala de espera\n", id);
        pthread_mutex_unlock(&print_mutex);

        sem_v(&customers);
        sem_p(&barbers);

        sem_v(&free_seats);
        pthread_mutex_lock(&print_mutex);
        printf("Cliente %d se está cortando el pelo\n", id);
        pthread_mutex_unlock(&print_mutex);
    } else {
        pthread_mutex_lock(&print_mutex);
        printf("Cliente %d se va (no hay sillas libres)\n", id);
        pthread_mutex_unlock(&print_mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {
    struct options opt;

    opt.barbers = 5;
    opt.customers = 1000;
    opt.cut_time = 3000;
    read_options(argc, argv, &opt);

    cut_time = opt.cut_time;

    if (sem_init(&customers, 0) != 0) {
        perror("sem_init customers");
        exit(1);
    }
    if (sem_init(&barbers, 0) != 0) {
        perror("sem_init barbers");
        exit(1);
    }
    if (sem_init(&free_seats, CHAIRS) != 0) {
        perror("sem_init free_seats");
        exit(1);
    }

    pthread_t *barber_tids = malloc(opt.barbers * sizeof(pthread_t));
    int *barber_ids = malloc(opt.barbers * sizeof(int));
    for (int i = 0; i < opt.barbers; i++) {
        barber_ids[i] = i + 1;
        pthread_create(&barber_tids[i], NULL, barber_thread, &barber_ids[i]);
    }

    pthread_t *customer_tids = malloc(opt.customers * sizeof(pthread_t));
    int *customer_ids = malloc(opt.customers * sizeof(int));
    for (int i = 0; i < opt.customers; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_tids[i], NULL, customer_thread, &customer_ids[i]);
    }

    for (int i = 0; i < opt.customers; i++) {
        pthread_join(customer_tids[i], NULL);
    }

    free(barber_tids);
    free(barber_ids);
    free(customer_tids);
    free(customer_ids);

    return 0;
}