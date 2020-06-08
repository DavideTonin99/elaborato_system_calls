/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"

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

void writeOutAck(Message *msg, Response *response)
{
//     if (response) {
//         char *path_to_fileout;
//         sprintf(path_to_fileout, "%s%d", "out_", msg->message_id);
//         sprintf(path_to_fileout, "%s%s", path_to_fileout, ".txt");
        
//         printf("Apertura file out '%s'...", path_to_fileout);
//         int fd_out = open(path_to_fileout, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
//         if (fd_out == -1)
//             ErrExit("open file out failed");

//         // for (Acknowledgment ack = response->ack; *ack != NULL; ++ack) {
//         //     // TODO scrittura su file
//         //     printf("%d\n", ack->timestamp);
//         // }

//         if (close(fd_out) == -1)
//             ErrExit("close file out failed");
//     }
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
