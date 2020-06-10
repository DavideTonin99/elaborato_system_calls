#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "time.h"
#include "sys/stat.h"
// INCLUDE MESSAGE QUEUE
#include "sys/msg.h"

#include "inc/err_exit.h"
#include "inc/defines.h"
#include "inc/ack_manager.h"
#include "inc/shared_memory.h"
#include "inc/semaphore.h"

Acknowledgment *shm_ptr_acklist;
int msq_id;

void ackManagerFreeResources() 
{
    if (msgctl(msq_id, IPC_RMID, NULL) == -1)
        ErrExit("<ack manager> remove message queue failed");
    freeSharedMemory(shm_ptr_acklist);
}

void ackManagerSigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT) {
        printf("<ack manager pid: %d>remove resources and exit...\n", getpid());
        ackManagerFreeResources();
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

void sendResponseToClient(int message_id) 
{
    Response response;
    response.mtype = message_id;
    int id_ack = 0;

    Acknowledgment ack_null = {0};

    for (int i = 0; i < SIZE_ACK_LIST; i++) {
        if (shm_ptr_acklist[i].message_id == message_id) {
            response.ack[id_ack++] = shm_ptr_acklist[i];
            shm_ptr_acklist[i] = ack_null;
        }
    }

    size_t size = sizeof(Response) - sizeof(long);
    if (msgsnd(msq_id, &response, size, 0) == -1)
        ErrExit("<ack manager> msgsnd send response to client failed");
}


void ackManagerRoutine() 
{
    for (int i = 0; i < SIZE_ACK_LIST; i++) {
        if (shm_ptr_acklist[i].message_id != 0) {
            if (contAckByMessageId(shm_ptr_acklist, shm_ptr_acklist[i].message_id) == N_DEVICES) {
                sendResponseToClient(shm_ptr_acklist[i].message_id);
            }
        }
    }
}

void execAckManager(int shmid_acklist, int msg_queue_key, int semid) 
{
    changeAckManagerSignalHandler();
    shm_ptr_acklist = (Acknowledgment *)getSharedMemory(shmid_acklist, 0);

    msq_id = msgget(msg_queue_key, IPC_CREAT | S_IWUSR | S_IRUSR);
    if (msq_id == -1)
        ErrExit("<ack manager> get message queue failed");

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
        ackManagerRoutine();
        semOp(semid, SEMNUM_ACKLIST, 1);
    }
}