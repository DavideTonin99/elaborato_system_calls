/// @file server.c
/// @brief Contiene l'implementazione del SERVER.
/*
Il server alloca i segmenti di memoria condivisa per la scacchiera e per la lista di acknowlwedgment,
e li rimuove prima di terminare.
Può essere terminato solo da un segnale SIGTERM.
Genera i semafori per sincronizzare l'accesso alle aree di memoria condivisa.
Genera i processi figli device e ack manager.
Gestisce il tempo per l'esecuzione dei movimenti dei device, ogni 2 secondi attiva il movimento
del primo device, che genera poi in cascata l'attivazione dei movimenti di tutti i device.
*/
#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "sys/wait.h"

// INCLUDE PROGETTO
#include "defines.h"
#include "err_exit.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "device.h"
#include "ack_manager.h"
#include "server.h"

int semid = -1;

// id shared memory
int shmid_board;
int shmid_acklist;
int shmid_deviceslist;

// message queue key
int msg_queue_key;

void freeResources()
{        
    coloredPrintf("red", 0, "<server> Free resources...\n");
    // chiusura di tutti i meccanismi di comunicazione/sincronizzazione tra processi
    removeSemaphoreSet(semid);
    removeSharedMemory(shmid_board);
    removeSharedMemory(shmid_acklist);
    removeSharedMemory(shmid_deviceslist);
}

void sigHandler(int sig)
{
    coloredPrintf("red", 0, "<server> Close processes and exit...\n");
    // terminazione processo server e figli
    if (killpg(getpgrp(), SIGTERM) == -1)
        ErrExit("kill failed");

    // attende che terminano tutti figli
    while (wait(NULL) != -1);

    freeResources();
    exit(0);
}

void changeSignalHandler()
{
    coloredPrintf("cyan", 1, "<server> Changing signal handler...\n");
    sigset_t signals_set;
    if (sigfillset(&signals_set) == -1)
        ErrExit("<server> sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExit("<server> sigprocmask failed");

    if (signal(SIGTERM, sigHandler) == SIG_ERR)
        ErrExit("<server> change signal handler failed");
}

void initDevices(int n_devices, const char *path_to_position_file)
{
    pid_t pid;
    for (int i = 0; i < n_devices; i++) {
        pid = fork();
        if (pid == -1) {
            coloredPrintf("red", 1, "device %d not created!", i);
            exit(0);
        } else if (pid == 0) {
            execDevice(i, semid, shmid_board, shmid_acklist, shmid_deviceslist, path_to_position_file);
        }
    }
}

void initAckManager()
{
    pid_t pid = fork();
    if (pid == -1) {
        coloredPrintf("red", 1, "ack_manager not created!");
        exit(0);
    } else if (pid == 0) {
        execAckManager(shmid_acklist, msg_queue_key, semid);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        coloredPrintf("red", 1, "Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        return 0;
    }

    // Legge e controlla la chiave della message queue
    msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        coloredPrintf("red", 1, "The message queue key must be greater than zero!\n");
        exit(1);
    }

    changeSignalHandler();

    coloredPrintf("cyan", 1, "<server> Initializing semaphores...\n");
    semid = initSemaphoreSet();

    coloredPrintf("cyan", 1, "<server> Initializing shared memory...\n");
    // Crea i segmenti di memoria condivisa
    shmid_board = allocSharedMemory(IPC_PRIVATE, sizeof(pid_t) * BOARD_ROWS * BOARD_COLS);
    shmid_acklist = allocSharedMemory(IPC_PRIVATE, sizeof(Acknowledgment) * SIZE_ACK_LIST);
    shmid_deviceslist = allocSharedMemory(IPC_PRIVATE, sizeof(pid_t) * N_DEVICES);

    initDevices(N_DEVICES, argv[2]);
    initAckManager();
    
    int i = 0;
    while (1) {
        sleep(2);
        i++;
        // sblocca la board
        semOp(semid, N_DEVICES, -1);
        coloredPrintf("yellow", 0, "\n# Step %d: device positions ########################\n", i);
    }

    return 0;
}