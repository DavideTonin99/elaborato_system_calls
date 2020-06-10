/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "time.h"
#include "string.h"

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

void writeOutAck(Message *msg, Response response)
{
    char path_to_fileout[25];
    sprintf(path_to_fileout, "out_%d.txt", msg->message_id);
    
    // printf("Apertura file out '%s'...", path_to_fileout);
    int fd_out = open(path_to_fileout, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_out == -1)
        ErrExit("open file out failed");

    char header[sizeof(msg->message) + 30];
    sprintf(header, "%s %d: %s\n", "Messaggio", msg->message_id, msg->message);
    if (write(fd_out, header, strlen(header)) == -1)
        ErrExit("write failed");

    char buffer[100];
    for (int i = 0; i < N_DEVICES; i++) {
        memset(buffer, 0, sizeof(buffer));   
        // TODO scrittura su file
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&response.ack[i].timestamp));
        sprintf(buffer, "%d, %d, %s\n", response.ack[i].pid_sender, response.ack[i].pid_receiver, timestamp);

        printf("%s", buffer);
        if (write(fd_out, buffer, strlen(buffer)) == -1)
            ErrExit("write failed");
    }

    if (close(fd_out) == -1)
        ErrExit("close file out failed");
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
