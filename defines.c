/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "stdio.h"
#include "time.h"

#include "defines.h"
#include "err_exit.h"

void printDebugMessage(Message *msg) 
{
    if (msg) {
        printf("\n------------------\n");
        printf("Debug Messaggio: \n");
        printf("pid_sender: %d\n", msg->pid_sender);
        printf("pid_receiver: %d\n", msg->pid_receiver);
        printf("message_id: %d\n", msg->message_id);
        printf("message: %s\n", msg->message);
        printf("max_distance: %lf\n", msg->max_distance);
        printf("------------------\n");
    } else {
        printf("Messaggio non valido!\n");
    }
}

int compareAcks (const void *a, const void *b) 
{
    Acknowledgment *first_ack = (Acknowledgment *)a;
    Acknowledgment *second_ack = (Acknowledgment *)b;

    return (first_ack->timestamp - second_ack->timestamp);
}

int checkAckAvailable(Acknowledgment *shm_ptr_acklist) 
{
    int result = 0;

    for (int i = 0; i < SIZE_ACK_LIST && result == 0; i++) {
        if (shm_ptr_acklist[i].message_id == 0) {
            result = 1;
        }
    }

    return result;
}

void addAck(Acknowledgment *shm_ptr_acklist, Message *msg) 
{
    Acknowledgment *ack = NULL;
    for (int i = 0; SIZE_ACK_LIST && ack == NULL; i++) {
        if (shm_ptr_acklist[i].message_id == 0) {
            ack = &shm_ptr_acklist[i];
            ack->pid_sender = msg->pid_sender;
            ack->pid_receiver = msg->pid_receiver;
            ack->message_id = msg->message_id;
            ack->timestamp = time(NULL);
            break;
        }
    }
}

int ackListContains(Acknowledgment *shm_ptr_acklist, pid_t pid_receiver, int message_id) 
{
    int result = 0;
    for (int i = 0; i < SIZE_ACK_LIST && !result; i++) {
        if (shm_ptr_acklist[i].message_id == message_id && shm_ptr_acklist[i].pid_receiver == pid_receiver)
            result = 1;
    }
    return result;
}

int contAckByMessageId(Acknowledgment *shm_ptr_acklist, int message_id) 
{
    int result = 0;
    for (int i = 0; i < SIZE_ACK_LIST; i++) {
        if (shm_ptr_acklist[i].message_id == message_id)
            result++;
    }
    return result;
}
