/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "sys/wait.h"

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "device.h"

int semid = -1;

// id shared memory
int board_shmid;
int acklist_shmid;

// ptr shared memory
pid_t *board_shm_ptr;
int *acklist_shm_ptr;

void freeResources()
{        
    printf("<server> Free resources...\n");
    // chiusura di tutti i meccanismi di comunicazione/sincronizzazione tra processi
    removeSemaphoreSet(semid);
    freeSharedMemory(board_shm_ptr);
    removeSharedMemory(board_shmid);
}

void sigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT) {
        printf("<server> Close processes and exit...\n");
        // terminazione processo server e figli
        kill(-getpid(), sig);
        
        // attende che terminano tutti figli
        while (wait(NULL) != -1);

        freeResources();
        exit(0);
    }
}

void changeSignalHandler()
{
    printf("<server> Changing signal handler...\n");
    sigset_t signals_set;
    if (sigfillset(&signals_set) == -1)
        ErrExit("<server> sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);
    sigdelset(&signals_set, SIGINT); // per DEBUG

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExit("<server> sigprocmask failed");

    if (signal(SIGTERM, sigHandler) == SIG_ERR)
        ErrExit("<server> change signal handler failed");
    // DEBUG
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        ErrExit("<server> change signal handler failed");
}

void initDevices(int n_devices, const char *path_to_position_file)
{
    pid_t pid;
    for (int i = 0; i < n_devices; i++) {
        pid = fork();
        if (pid == -1) {
            printf("child %d not created!", i);
            exit(0);
        } else if (pid == 0) {
            execDevice(i, semid, board_shmid, path_to_position_file);
        }
    }

    // genera l'ack_manager
    // if (pid != 0) {
    //     pid = fork();
    //     if (pid == -1) {
    //         ErrExit("ack_manager not created!");
    //     } else if (pid == 0) {
    //         // ack_manager
    //     }
    // }
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

    printf("<server> Inizializzazione semafori...\n");
    semid = initSemaphoreSet(N_DEVICES+2, N_DEVICES);

    printf("<server> Inizializzazione memoria condivisa...\n");
    // Crea i segmenti di memoria condivisa
    board_shmid = allocSharedMemory(IPC_PRIVATE, sizeof(int) * BOARD_ROWS * BOARD_COLS);
    board_shm_ptr = (pid_t *)getSharedMemory(board_shmid, 0);

    initDevices(N_DEVICES, argv[2]);
    
    int i = 0;
    while (1) {
        printf("# Step %d: device positions ########################\n", i);
        // sblocca la board
        semOp(semid, N_DEVICES, -1);
        sleep(2);
        i++;
    }

    return 0;
}