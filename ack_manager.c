#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "time.h"

#include "err_exit.h"
#include "defines.h"
#include "ack_manager.h"
#include "shared_memory.h"
#include "semaphore.h"

Acknowledgment *shm_ptr_acklist;

void ackManagerSigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT) {
        printf("<ack manager pid: %d>remove resources and exit...\n", getpid());
        freeSharedMemory(shm_ptr_acklist);
        exit(0);
    }
}

void changeAckManagerSignalHandler() 
{
    // printf("<ack manager pid: %d, id: %d> Changing signal handler...\n", getpid(), id_ack manager);
    sigset_t signals_set;
    if (sigfillset(&signals_set) == -1)
        ErrExit("<ack manager> sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);
    sigdelset(&signals_set, SIGINT); // per DEBUG

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExit("<ack manager> sigprocmask failed");

    if (signal(SIGTERM, ackManagerSigHandler) == SIG_ERR)
        ErrExit("<ack manager> change signal handler failed");

    // DEBUG
    if (signal(SIGINT, ackManagerSigHandler) == SIG_ERR)
        ErrExit("<ack manager> change signal handler failed");
}

void execAckManager(int shmid_acklist, int msg_queue_key, int semid) 
{
    changeAckManagerSignalHandler();
    shm_ptr_acklist = (Acknowledgment *)getSharedMemory(shmid_acklist, 0);
    while(1) {
        sleep(5);
        semOp(semid, SEMNUM_ACKLIST, -1);

        // DEBUG
        printf("Ack list:\n");
        for (int i = 0; i < SIZE_ACK_LIST; i++) {
            if (shm_ptr_acklist[i].message_id != 0) {
                Acknowledgment ack = shm_ptr_acklist[i];
                char buff[20];
                strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&ack.timestamp));

                printf("<ack %d> %d %d %d %s\n", i, ack.pid_sender, ack.pid_receiver, ack.message_id, buff);
            }
        }
        semOp(semid, N_DEVICES+1, 1);
    }
}