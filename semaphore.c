/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include "err_exit.h"
#include "semaphore.h"

void semOp(int semid, unsigned short sem_num, short sem_op)
{
    struct sembuf semaphore_op = {
        .sem_num = sem_num,
        .sem_op = sem_op,
        .sem_flg = 0};

    if (semop(semid, &semaphore_op, 1) == -1)
        ErrExit("semop failed");
}