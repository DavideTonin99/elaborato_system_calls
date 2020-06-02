/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "fcntl.h"

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

int semid = -1;
// id shared memory
int board_shmid;
int acklist_shmid;
// ptr shared memory
int *board_shm_ptr;
int *acklist_shm_ptr;

void sigHandler(int sig)
{
    if (sig == SIGTERM) {
        // TODO: chiusura di tutti i meccanismi di comunicazione/sincronizzazione tra processi
        removeSemaphoreSet(semid);
        freeSharedMemory(board_shmid);
        removeSharedMemory(board_shm_ptr);

        printf("Server close processes and exit...\n", getpid());

        // terminazione processo server e figli
        kill(-getpid(), sig);
        exit(0);
    }
}

void changeSignalHandler()
{
    sigset_t signals_set;
    if (sigfillset(&signals_set) == -1)
        ErrExit("sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExit("sigprocmask failed");

    if (signal(SIGTERM, sigHandler) == SIG_ERR)
        ErrExit("change signal handler failed");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        return 0;
    }

    // Legge e controlla la chiave della message queue
    int msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        printf("The message queue key must be greater than zero!\n");
        exit(1);
    }

    changeSignalHandler();

    // Apre il file posizioni
    int file_pos = open(argv[2], O_RDONLY);
    if (file_pos == -1)
        ErrExit("open file posizioni failed");

    semid = initSemaphoreSet(N_DEVICES+2, N_DEVICES);

    // Crea i segmenti di memoria condivisa
    board_shmid = allocSharedMemory(IPC_PRIVATE, sizeof(int) * ROWS * COLS);
    board_shm_ptr = getSharedMemory(board_shmid, 0);

    return 0;
}