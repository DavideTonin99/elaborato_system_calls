/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include "stdio.h"
#include "fcntl.h"

#include "err_exit.h"
#include "semaphore.h"

void semOp(int semid, unsigned short sem_num, short sem_op)
{
    struct sembuf semaphore_op = {
        .sem_num = sem_num,
        .sem_op = sem_op,
        .sem_flg = 0
    };

    if (semop(semid, &semaphore_op, 1) == -1)
        ErrExit("semop failed");
}

int initSemaphoreSet(int length, int devices) 
{
    // Crea un set di 'length' semafori
    int semid = semget(IPC_PRIVATE, length, S_IRUSR | S_IWUSR);
    if (semid == -1)
        ErrExit("semget failed");

    // Inizializza i semafori
    // I primi 'devices' semafori sono per i device
    // Il semaforo successivo all'ultimo device gestisce l'accesso alla scacchiera
    // Il penultimo semaforo gestisce l'accesso alla lista di acknowledgments
    // L'ultimo semaforo gestisce l'accesso alla lista dei device
    unsigned short sem_init_values[length];
    int i = 0;
    sem_init_values[i] = 1; // il primo device a 1

    // tutti gli altri device a 0
    for (i = 1; i < devices; i++)
        sem_init_values[i] = 0;
    // board: 1 -> bloccato, 0 -> sbloccato
    sem_init_values[i] = 1;
    sem_init_values[i+1] = 1;
    sem_init_values[i+2] = 1;
    union semun arg;
    arg.array = sem_init_values;

    if (semctl(semid, 0, SETALL, arg))
        ErrExit("semctl SETALL failed");

    return semid;
}

void removeSemaphoreSet(int semid)
{
    if (semctl(semid, 0 /* ignored */, IPC_RMID, NULL) == -1)
        ErrExit("semctl IPC_RMID failed");
}