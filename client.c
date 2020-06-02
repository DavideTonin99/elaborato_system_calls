/// @file client.c
/// @brief Contiene l'implementazione del client.
/*
Un client può connettersi ad un device per mezzo della fifo dev_fifo.pid, 
gestita dal device stesso (pid è il pid del device), 
per passargli un messaggio contenente:
- il proprio pid (pid_sender)
- il pid del device ricevente (pid_receiver)
- un id del messaggio (message_id)
- una stringa di testo (message)
- un numero positivo (max_dist).

Quando il client riceve il messaggio nella coda di messaggi genera un file out_message_id.txt
(dove message_id è l’identificativo del messaggio) e vi scrive la lista di 5 acknowledgement che
identificano i passaggi fatti dal messaggio con i relativi istanti di tempo.
Una volta generato il file di output il client termina.
*/

// INCLUDE GENERICI
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "sys/stat.h"

// INCLUDE MESSAGE QUEUE
#include "sys/msg.h"

// INCLUDE FILE
#include "fcntl.h"
#include "unistd.h"

#include "err_exit.h"
#include "defines.h"

char *base_path_to_device_fifo = "/tmp/dev_fifo.";

int readInt(const char *buffer) 
{
    char *endptr = NULL;
    errno = 0;
    long int res = strtol(buffer, &endptr, 10);

    if (errno != 0 || *endptr != '\n' || res < 0) {
        printf("invalid input argument\n");
        exit(1);
    }

    return res;
}

double readDouble(const char *buffer) 
{
    char *endptr = NULL;
    errno = 0;
    double res = strtod(buffer, &endptr);

    if (errno != 0 || *endptr != '\n' || res < 0) {
        printf("invalid input argument\n");
        exit(1);
    }
    
    return res;
}

void sendMessage(Message *msg)
{
    // Apre la FIFO per inviare il messaggio al device
    char *path_to_device_fifo;
    sprintf(path_to_device_fifo, "%s%d", base_path_to_device_fifo, msg->pid_receiver);
    
    int device_fifo = open(path_to_device_fifo, O_WRONLY);
    if (device_fifo == -1)
        ErrExit("open device fifo failed");
    
    printf("Sending the message %d to device %d...", msg->message_id, msg->pid_receiver);
    // Invia il messaggio al device
    if (write(device_fifo, msg, sizeof(Message)) != sizeof(Message))
        ErrExit("write message failed");
    
    if (close(device_fifo))
        ErrExit("close device fifo failed");
}

int main(int argc, char *argv[])
{
    // Controlla gli argomenti da linea di comando
    if (argc < 2 || argc > 3) {
        printf("Usage: %s msg_queue_key [input_filename]\n", argv[0]);
        exit(1);
    }

    int fd_input = NULL;
    if (argc == 3) {
        // se c'è il file in input, lo usa come standard input
        close(STDIN_FILENO);
        fd_input = open(argv[2], O_RDONLY);
        if (fd_input == -1)
            ErrExit("open input file failed");
    }

    // Legge e controlla la chiave della message queue
    int msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        printf("The message queue key must be greater than zero!\n");
        exit(1);
    }

    // Crea un messaggio
    Message msg;

    char buffer[10];
    size_t len;

    // Legge il pid del device ricevente
    if (fd_input)
        printf("Lettura input messaggio dal file %s ...\n", argv[2]);

    if (!fd_input)
        printf("Insert pid receiver device: ");
    fgets(buffer, sizeof(buffer), stdin);
    msg.pid_receiver = readInt(buffer);

    if (!fd_input)
        printf("Insert message id: ");
    fgets(buffer, sizeof(buffer), stdin);
    msg.message_id = readInt(buffer);

    if (!fd_input)
        printf("Insert message: ");
    fgets(msg.message, sizeof(msg.message), stdin);
    len = strlen(msg.message);
    msg.message[len - 1] = '\0';

    if (!fd_input)
        printf("Insert max distance: ");
    fgets(buffer, sizeof(buffer), stdin);
    msg.max_distance = readDouble(buffer);

    // Imposta il pid del processo sender
    msg.pid_sender = getpid();

    printDebugMessage(&msg);
    sendMessage(&msg);
    
    // Get identificatore message queue
    int msq_id = msgget(msg_queue_key, S_IRUSR | S_IWUSR);
    if (msq_id == -1)
        ErrExit("msgget failed");

    Response response;
    size_t mSize = (sizeof(Acknowledgment)*N_DEVICES) - sizeof(long);
    // di default, msgrcv è bloccante quindi se non ci sono messaggi si ferma ad aspettare
    // aspetta la risposta del server
    if (msgrcv(msq_id, &response, mSize, msg.message_id, 0) == -1)
        ErrExit("msgrcv failed");

    writeOutAck(&msg, &response);

    if (fd_input)
        if (close(fd_input))
            ErrExit("close file input failed");

    return 0;
}