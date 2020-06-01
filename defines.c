/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "stdio.h"
#include "defines.h"

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