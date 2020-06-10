/// @file device.c
/// @brief Contiene l'implementazione del DEVICE
/*
Un device viene generato dal server.

*/

#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "string.h"
#include "time.h"

#include "inc/defines.h"
#include "inc/err_exit.h"
#include "inc/semaphore.h"
#include "inc/shared_memory.h"
#include "inc/fifo.h"
#include "inc/device.h"

// memoria condivisa
pid_t *shm_ptr_board;
Acknowledgment *shm_ptr_acklist;

// device
int id_device; 

// device FIFO
const char *base_path_to_device_fifo = "/tmp/dev_fifo.";
char path_to_device_fifo[25];
int fd_device_fifo;

// file descriptor file posizioni device
int fd_position;

void freeDeviceResources() 
{
    freeSharedMemory(shm_ptr_board);
    freeSharedMemory(shm_ptr_acklist);

    if (close(fd_device_fifo) == -1)
        ErrExit("<device> close device fifo failed");
    if(unlink(path_to_device_fifo) == -1)
        ErrExit("<device> unlink fifo failed");
    if(close(fd_position) == -1)
        ErrExit("<device> close position file failed");

}

void deviceSigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT) {
        printf("<device pid: %d>remove resources and exit...\n", getpid());
        freeDeviceResources();
        exit(0);
    }
}

void changeDeviceSignalHandler() 
{
    // printf("<device pid: %d, id: %d> Changing signal handler...\n", getpid(), id_device);
    sigset_t signals_set;
    if (sigfillset(&signals_set) == -1)
        ErrExit("<device> sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);
    sigdelset(&signals_set, SIGINT); // per DEBUG

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExit("<device> sigprocmask failed");

    if (signal(SIGTERM, deviceSigHandler) == SIG_ERR)
        ErrExit("<device> change signal handler failed");

    // DEBUG
    if (signal(SIGINT, deviceSigHandler) == SIG_ERR)
        ErrExit("<device> change signal handler failed");
}

void readNextLine(char *next_line) 
{
    // contiene la riga successiva
    char row[ROW_BUFFER_SIZE] = {0};
    // contiene byte successivo
    char buffer[2] = {0};

    while(read(fd_position, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)
        strcat(row, buffer);

    // DEBUG: se è finito il file, ricomincia da capo
    if(strlen(row) == 0){
        lseek(fd_position, 0, SEEK_SET); 
        readNextLine(next_line);
    } else {
        memcpy(next_line, row, strlen(row)+1);
    }
}

void getNextPosition(char *next_line, Position *position)
{
    char *pos = strtok(next_line, "|");
    for (int cont_pipes = 0; cont_pipes < id_device; cont_pipes++) {
        pos = strtok(NULL, "|");
    }

    char *row = strtok(pos, ",");
    char *col = strtok(NULL, ",");

    position->row = atoi(row);
    position->col = atoi(col);
}

void waitTurnAndBoard(int semid) 
{
    // il device aspetta il proprio turno
    semOp(semid, (unsigned short)id_device, -1);
    // il device aspetta che la board sia accessibile
    semOp(semid, (unsigned short)SEMNUM_BOARD, 0);
}

void signalEndTurn(int semid)
{
    if (id_device < N_DEVICES - 1) {
        // se il device corrente non è l'ultimo, sblocca il prossimo
        semOp(semid, (unsigned short)(id_device + 1), 1);
    } else {
        // blocca la board
        semOp(semid, (unsigned short)SEMNUM_BOARD, 1);
        // sblocca il primo device
        semOp(semid, 0, 1);
    }
}

int checkAckAvailable() 
{
    int result = 0;

    for (int i = 0; i < SIZE_ACK_LIST && result == 0; i++) {
        if (shm_ptr_acklist[i].message_id == 0) {
            result = 1;
        }
    }

    return result;
}

void addAck(Message *msg) 
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

int ackListContains(pid_t pid_receiver, int message_id) 
{
    int result = 0;
    for (int i = 0; i < SIZE_ACK_LIST && !result; i++) {
        if (shm_ptr_acklist[i].message_id == message_id && shm_ptr_acklist[i].pid_receiver == pid_receiver)
            result = 1;
    }
    return result;
}

pid_t searchAvailableDevice(Position *position, int message_id, double max_distance) 
{
    pid_t result = 0;
    int row_min = (position->row - max_distance) > 0 ? (position->row - max_distance) : 0;
    int col_min = (position->col - max_distance) > 0 ? (position->col - max_distance) : 0;

    int row_max = (position->row + max_distance + 1) < BOARD_ROWS ? (position->col + max_distance + 1) : BOARD_ROWS;
    int col_max = (position->col + max_distance + 1) < BOARD_COLS ? (position->col + max_distance + 1) : BOARD_COLS;

    for (int row = row_min; row < row_max && result == 0; row++) {
        for (int col = col_min; col < col_max && result == 0; col++) {
            int offset = row*BOARD_COLS+col;
            if (shm_ptr_board[offset] > 0 && shm_ptr_board[offset] != getpid() && !ackListContains(shm_ptr_board[offset], message_id))
                result = shm_ptr_board[offset];
        }
    }
    return result;
}

void sendMessages(Position *position, Message *messages_buffer, int *n_messages, int semid)
{
    pid_t pid_next_device = 0;
    char path_to_receiver_device_fifo[25];
    
    for (int i = 0; i < *n_messages; i++) {
        semOp(semid, (unsigned short)SEMNUM_ACKLIST, -1);
        if (checkAckAvailable())
            pid_next_device = searchAvailableDevice(position, messages_buffer[i].message_id, messages_buffer[i].max_distance);
        semOp(semid, (unsigned short)SEMNUM_ACKLIST, 1);

        if (pid_next_device != 0) {
            sprintf(path_to_receiver_device_fifo, "%s%d", base_path_to_device_fifo, pid_next_device);
            int fd_receiver_device_fifo = openFIFO(path_to_receiver_device_fifo, O_WRONLY);

            Message msg = messages_buffer[i];
            msg.pid_sender = getpid();
            msg.pid_receiver = pid_next_device;

            writeFIFO(fd_receiver_device_fifo, &msg);
            // shift a sinistra per cancellare il messaggio appena inviato
            for (int j = i; j < *n_messages && j < MSG_BUFFER_SIZE-1; j++)
                messages_buffer[j] = messages_buffer[j+1];
            (*n_messages)--;

            closeFIFO(fd_receiver_device_fifo);
        }
    }
}

void readMessages(Message *messages_buffer, int *n_messages, int semid) 
{
    int bR;
    Message msg;

    do {
        bR = -1;
        semOp(semid, (unsigned short)SEMNUM_ACKLIST, -1); // blocca la ack list
        int available = checkAckAvailable();
        semOp(semid, (unsigned short)SEMNUM_ACKLIST, 1); // sblocca la ack list

        if (available) {
            bR = read(fd_device_fifo, &msg, sizeof(Message));
            if (bR == -1) {
                ErrExit("<device> read fifo failed");
            } else if (bR == sizeof(Message)) {
                // printf("<device %d> Read:\n", getpid());
                // printDebugMessage(&msg);
                semOp(semid, (unsigned short)SEMNUM_ACKLIST, -1); // blocca la ack list
                addAck(&msg);
                if (contAckByMessageId(shm_ptr_acklist, msg.message_id) < N_DEVICES) {
                    messages_buffer[*n_messages] = msg;
                    (*n_messages)++;
                }
                semOp(semid, (unsigned short)SEMNUM_ACKLIST, 1); // sblocca la ack list
            }
        }
    } while (bR > 0);
}

void moveDevice(Position *position, char *next_line) 
{
    int old_position_index = position->row * BOARD_COLS + position->col;

    readNextLine(next_line);
    getNextPosition(next_line, position);

    int next_position_index = position->row * BOARD_COLS + position->col;

    // se la nuova posizione non è occupata, sposta il device
    if (shm_ptr_board[next_position_index] == 0) {
        // prima libera la precedente zona occupata
        shm_ptr_board[old_position_index] = 0;
        // occupa la nuova zona
        shm_ptr_board[next_position_index] = getpid();
    } else {
        // altrimenti il device resta fermo, e ripristina la posizione
        position->row = old_position_index / BOARD_COLS;
        position->col = old_position_index % BOARD_COLS;
    }
}

void printDebugDevice(Position position, int n_messages, Message *messages_buffer) 
{
    printf("%d %d %d msgs: ", getpid(), position.row, position.col);
    // stampa lista messages
    for (int i = 0; i < n_messages; i++)
        printf("%d, ", messages_buffer[i].message_id);
    printf("\n");
    if (id_device == N_DEVICES - 1) {
        printf("####################################################\n");
    }
}

void execDevice(int _id_device, int semid, int shmid_board, int shmid_acklist, const char *path_to_position_file) 
{
    id_device = _id_device;

    changeDeviceSignalHandler();

    shm_ptr_board = (pid_t *)getSharedMemory(shmid_board, 0);
    shm_ptr_acklist = (Acknowledgment *)getSharedMemory(shmid_acklist, 0);

    sprintf(path_to_device_fifo, "%s%d", base_path_to_device_fifo, getpid());
    createFIFO(path_to_device_fifo, S_IRUSR | S_IWUSR);
    
    fd_device_fifo = openFIFO(path_to_device_fifo, O_RDONLY | O_NONBLOCK);

    fd_position = open(path_to_position_file, O_RDONLY);
    if (fd_position == -1)
        ErrExit("<device> open position file failed");

    // posizione device
    Position position = {0,0};
    char next_line[ROW_BUFFER_SIZE];

    // buffer di ricezione dei messaggi
    Message messages_buffer[MSG_BUFFER_SIZE] = {};
    int n_messages = 0;

    while(1) {
        // aspetta il turno e l'accesso alla scacchiera
        waitTurnAndBoard(semid);
        // se ha messaggi nel buffer di ricezione, prova ad inviarli
        sendMessages(&position, messages_buffer, &n_messages, semid);
        // legge i messaggi dalla FIFO
        readMessages(messages_buffer, &n_messages, semid);
        // effettua il movimento
        moveDevice(&position, next_line);
        // stampa la riga di debug
        printDebugDevice(position, n_messages, messages_buffer);
        // segnala la fine del turno (liberando i semafori opportuni)
        signalEndTurn(semid);
    }
}