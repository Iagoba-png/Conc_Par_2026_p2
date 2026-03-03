#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "options.h"
#include "sem.h"

#define CHAIRS 3               // número de sillas en la sala de espera

sem_t customers;               // clientes esperando (para despertar barbero)
sem_t barbers;                 // barberos listos para atender
sem_t free_seats;              // sillas libres en la sala de espera

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; // para salida ordenada

int cut_time;                  // tiempo de corte (microsegundos)

void* barber_thread(void* arg) {
    int id = *(int*)arg;
    while (1) {
        sem_p(&customers);              // espera hasta que llegue un cliente
        sem_v(&barbers);                 // indica que está listo para atender
        pthread_mutex_lock(&print_mutex);
        printf("Barbero %d corta el pelo\n", id);
        pthread_mutex_unlock(&print_mutex);
        usleep(cut_time);                // simula el corte
    }
    return NULL;
}

void* customer_thread(void* arg) {
    int id = *(int*)arg;
    // Llegada aleatoria para evitar ráfagas
    usleep(rand() % 1000000);

    if (sem_tryp(&free_seats) == 0) {    // intenta ocupar una silla de espera
        pthread_mutex_lock(&print_mutex);
        printf("Cliente %d entra a la sala de espera\n", id);
        pthread_mutex_unlock(&print_mutex);

        sem_v(&customers);                // avisa al barbero
        sem_p(&barbers);                   // espera a que un barbero lo atienda

        sem_v(&free_seats);                 // libera la silla de espera
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

    // Valores por defecto
    opt.barbers = 5;
    opt.customers = 1000;
    opt.cut_time = 3000;
    read_options(argc, argv, &opt);

    cut_time = opt.cut_time;

    // Inicializar semáforos
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

    // Crear hilos de barberos
    pthread_t *barber_tids = malloc(opt.barbers * sizeof(pthread_t));
    int *barber_ids = malloc(opt.barbers * sizeof(int));
    for (int i = 0; i < opt.barbers; i++) {
        barber_ids[i] = i + 1;
        pthread_create(&barber_tids[i], NULL, barber_thread, &barber_ids[i]);
    }

    // Crear hilos de clientes
    pthread_t *customer_tids = malloc(opt.customers * sizeof(pthread_t));
    int *customer_ids = malloc(opt.customers * sizeof(int));
    for (int i = 0; i < opt.customers; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_tids[i], NULL, customer_thread, &customer_ids[i]);
    }

    // Esperar a que todos los clientes terminen
    for (int i = 0; i < opt.customers; i++) {
        pthread_join(customer_tids[i], NULL);
    }

    // Liberar memoria (los barberos siguen ejecutándose, pero el proceso termina al salir de main)
    free(barber_tids);
    free(barber_ids);
    free(customer_tids);
    free(customer_ids);

    return 0;
}