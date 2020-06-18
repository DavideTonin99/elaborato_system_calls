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
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "sys/stat.h"
#include "sys/msg.h"
#include "fcntl.h"
#include "unistd.h"
#include "time.h"
#include "errno.h"
#include "signal.h"

// INCLUDE PROGETTO
#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "client.h"

const char *base_path_to_device_fifo = "/tmp/dev_fifo.";

void sigHandler(int sig)
{
    if (sig == SIGUSR1) {
        coloredPrintf("red", 1, "Client %d ERROR: MESSAGE ID NON UNIVOCO !!!\n", getpid());
        exit(1);
    }
}

void changeSignalHandler() 
{
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        ErrExit("change signal handler failed");
}

int readInt(const char *buffer) 
{
    char *endptr = NULL;
    errno = 0;
    long int res = strtol(buffer, &endptr, 10);

    if (errno != 0 || *endptr != '\n' || res < 1) {
        coloredPrintf("red", 1, "<read int> invalid input argument\n");
        exit(1);
    }

    return res;
}

double readDouble(const char *buffer) 
{
    char *endptr = NULL;
    errno = 0;
    double res = strtod(buffer, &endptr);

    if (errno != 0 || *endptr != '\n' || res < 1) {
        coloredPrintf("red", 1, "<read double> invalid input argument\n");
        exit(1);
    }
    
    return res;
}

void sendMessage(Message *msg)
{
    // Apre la FIFO per inviare il messaggio al device
    char path_to_device_fifo[25];
    sprintf(path_to_device_fifo, "%s%d", base_path_to_device_fifo, msg->pid_receiver);
    
    int fd_device_fifo = openFIFO(path_to_device_fifo, O_WRONLY);
    
    coloredPrintf("green", 1, "Sending the message %d to device %d...\n", msg->message_id, msg->pid_receiver);
    writeFIFO(fd_device_fifo, msg);
    closeFIFO(fd_device_fifo);
}

void writeOutAck(Message *msg, Response response)
{
    if (mkdir("out/", S_IRUSR | S_IWUSR | S_IXUSR) == -1 && errno != EEXIST)
        ErrExit("mkdir failed");

    char path_to_fileout[25];
    sprintf(path_to_fileout, "out/out_%d.txt", msg->message_id);
    
    int fd_out = open(path_to_fileout, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_out == -1)
        ErrExit("open file out failed");

    char header[strlen(msg->message) + 30];
    sprintf(header, "%s %d: %s\n", "Messaggio", msg->message_id, msg->message);
    if (write(fd_out, header, strlen(header)) == -1)
        ErrExit("write failed");

    char buffer[100] = "Lista acknowledgements:\n";
    if (write(fd_out, buffer, strlen(buffer)) == -1)
        ErrExit("write failed");

    for (int i = 0; i < N_DEVICES; i++) {
        memset(buffer, 0, sizeof(buffer));   
        // TODO scrittura su file
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&response.ack[i].timestamp));
        sprintf(buffer, "%d, %d, %s\n", response.ack[i].pid_sender, response.ack[i].pid_receiver, timestamp);

        if (write(fd_out, buffer, strlen(buffer)) == -1)
            ErrExit("write failed");
    }

    if (close(fd_out) == -1)
        ErrExit("close file out failed");
}

int main(int argc, char *argv[])
{
    // Controlla gli argomenti da linea di comando
    if (argc != 2 && argc != 6) {
        coloredPrintf("red", 1, "Usage: %s msg_queue_key \nor\n%s msg_queue_key pid_receiver message_id message max_distance\n", argv[0], argv[0]);
        exit(1);
    }

    changeSignalHandler();

    // Legge e controlla la chiave della message queue
    int msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        coloredPrintf("red", 1, "The message queue key must be greater than zero!\n");
        exit(1);
    }

    // Crea un messaggio
    Message msg;

    if (argc == 6) {
        msg.pid_receiver = atoi(argv[2]);
        msg.message_id = atoi(argv[3]);
        
        memcpy(msg.message, argv[4], strlen(argv[4]));
        size_t len = strlen(argv[4]);
        msg.message[len] = '\0';

        msg.max_distance = atof(argv[5]);

        if(msg.pid_receiver < 1 || msg.message_id < 0 || msg.max_distance < 1){
            coloredPrintf("red", 0, "<client %d> script input < 0\n", getpid());
            exit(1);
        }
    } else {
        char buffer[10];
        size_t len;

        coloredPrintf("green", 1, "Insert pid receiver device: ");
        fgets(buffer, sizeof(buffer), stdin);
        msg.pid_receiver = readInt(buffer);

        coloredPrintf("green", 1, "Insert message id: ");
        fgets(buffer, sizeof(buffer), stdin);
        msg.message_id = readInt(buffer);

        coloredPrintf("green", 1, "Insert message: ");
        fgets(msg.message, sizeof(msg.message), stdin);
        len = strlen(msg.message);
        msg.message[len - 1] = '\0';

        coloredPrintf("green", 1, "Insert max distance: ");
        fgets(buffer, sizeof(buffer), stdin);
        msg.max_distance = readDouble(buffer);
    }

    // Imposta il pid del processo sender
    msg.pid_sender = getpid();

    printDebugMessage(&msg);
    sendMessage(&msg);
    
    // Get identificatore message queue
    int msq_id = msgget(msg_queue_key, S_IRUSR | S_IWUSR);
    if (msq_id == -1)
        ErrExit("msgget failed");

    Response response;
    size_t size = sizeof(Response) - sizeof(long);
    // di default, msgrcv è bloccante quindi se non ci sono messaggi si ferma ad aspettare
    // aspetta la risposta del server
    if (msgrcv(msq_id, &response, size, msg.message_id, 0) == -1)
        ErrExit("msgrcv failed");

    writeOutAck(&msg, response);
    coloredPrintf("cyan", 0, "client %d received response for message %d and exit...\n", getpid(), msg.message_id);

    return 0;
}