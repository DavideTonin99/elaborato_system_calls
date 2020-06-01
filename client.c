/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "stdio.h"
#include "stdlib.h"
#include "sys/msg.h"
#include "string.h"
#include "sys/stat.h"
#include <fcntl.h>
#include <unistd.h>

#include "err_exit.h"
#include "defines.h"

char *base_path_to_server_fifo = "/tmp/dev_fifo.";

int main(int argc, char *argv[])
{
    // Controlla gli argomenti da linea di comando
    if (argc < 2 || argc > 3) {
        printf("Usage: %s msg_queue_key [input_filename]\n", argv[0]);
        exit(1);
    }

    char *input_filename;
    if (argc == 3) 
    {
        // se c'è il file in input, lo usa come standard input
        close(STDIN_FILENO);
        int fd = open(argv[2], O_RDONLY);
        if (fd == -1)
            ErrExit("open input file failed");
    }

    // Legge e controlla la chiave della message queue
    int msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        printf("The message queue key must be greater than zero!\n");
        exit(1);
    }

    // Get identificatore message queue
    int msq_id = msgget(msg_queue_key, S_IRUSR | S_IWUSR);
    if (msq_id == -1)
        ErrExit("msgget failed");

    // Crea un messaggio
    Message msg;

    printf("Insert pid receiver device: ");
    scanf();
    printf("Insert message id: ");
    printf("Insert message: ");
    printf("Insert max distance: ");

    // Imposta il pid del processo sender
    msg.pid_sender = getpid();

    return 0;
}