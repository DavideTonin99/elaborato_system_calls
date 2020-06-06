#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "string.h"

#include "defines.h"
#include "err_exit.h"
#include "device.h"
#include "semaphore.h"
#include "shared_memory.h"

int id_device; 

const char *base_path_to_device_fifo = "/tmp/dev_fifo.";
char path_to_device_fifo[25];
int fd_device_fifo;
int position_fd;

pid_t *board_shm_ptr;

Position next_position = {0,0};
// buffer di ricezione dei messaggi
Message *messages_buffer[MSG_BUFFER_SIZE] = {NULL};

void freeDeviceResources() 
{
    if (close(fd_device_fifo) == -1)
        ErrExit("<device> close device fifo failed");
    if(unlink(path_to_device_fifo) == -1)
        ErrExit("<device> unlink fifo failed");
    if(close(position_fd) == -1)
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
    // ogni valore può essere di due cifre, due per ogni device, per N_DEVICES + le pipe come divisori di coordinate
    char row[ROW_BUFFER_SIZE] = {0};
    // contiene byte successivo
    char buffer[2] = {0};

    while(read(position_fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)
        strcat(row, buffer);

    // se è finito il file, ricomincia da capo    
    if(strlen(row) == 0){
        lseek(position_fd, 0, SEEK_SET); 
        readNextLine(next_line);
    } else {
        memcpy(next_line, row, strlen(row)+1);
    }
}

void getNextPosition(char *next_line, Position *next_position)
{
    for (int cont_pipes = 0; cont_pipes < id_device && *next_line != '\0'; next_line++) {
        if (*next_line == '|')
            cont_pipes++;
    }

    // prende il valore della riga
    char buffer[ROW_BUFFER_SIZE];
    int index;
    
    for(index = 0; *next_line != ','; next_line++, index++)
        buffer[index] = *next_line;
    
    next_position->row = atoi(buffer);
    // salta la virgola
    next_line++; 

    // reset buffer
    memset(buffer, 0, sizeof(buffer));     
    
    // prende il valore della colonna
    for(index = 0; *next_line != '\0' && *next_line != '|'; next_line++, index++)
        buffer[index] = *next_line;
    
    next_position->col = atoi(buffer);
}

void waitTurnAndBoard(int semid) 
{
    // il device aspetta il proprio turno
    semOp(semid, (unsigned short)id_device, -1);
    // il device aspetta che la board sia accessibile
    semOp(semid, (unsigned short)N_DEVICES, 0);
}

void signalEndTurn(int semid)
{
    if (id_device < N_DEVICES - 1) {
        // se il device corrente non è l'ultimo, sblocca il prossimo
        semOp(semid, (unsigned short)(id_device + 1), 1);
    } else {
        // blocca la board
        semOp(semid, (unsigned short)N_DEVICES, 1);
        // sblocca il primo device
        semOp(semid, 0, 1);
    }
}

pid_t searchAvailableDevice(Message *msg) 
{
    pid_t result = 0;
    for (int row = (next_position.row - msg->max_distance); row < (next_position.row + msg->max_distance) && row < BOARD_ROWS && result == 0; row++) {
        for (int col = (next_position.col - msg->max_distance); col < (next_position.col + msg->max_distance) && col < BOARD_COLS && result == 0; col++) {
            int offset = row*BOARD_COLS+col;
            if (board_shm_ptr[offset] != 0)
                result = board_shm_ptr[offset];
        }
    }
    return result;
}

void sendMessages()
{
    pid_t next_device;
    char path_to_receiver_device_fifo[25];
    
    for (int i = 0; i < sizeof(messages_buffer) && messages_buffer[i]; i++) {
        next_device = searchAvailableDevice(messages_buffer[i]);
        if (next_device != 0) {
            sprintf(path_to_receiver_device_fifo, "%s%d", base_path_to_device_fifo, next_device);
            if( access(path_to_receiver_device_fifo, F_OK ) != -1 ) {
                int fd_receiver_device_fifo = open(path_to_receiver_device_fifo, O_WRONLY);
                if (fd_receiver_device_fifo == -1) {
                    printf("<device %d> open fifo receiver device (pid = %d) failed\n", getpid(), next_device);
                } else {
                    int res = write(fd_receiver_device_fifo, messages_buffer[i], sizeof(Message));
                    if (res == -1 || res != sizeof(Message)) {
                        printf("<device %d> write to fifo receiver device (pid = %d) failed\n", getpid(), next_device);
                    } else {
                        // libera il posto nel buffer di ricezione
                        messages_buffer[i] = NULL;
                    }
                }
            }
        }
    }
}

void readMessages() 
{
    int bR = -1;
    Message msg;
    int i = 0;

    do {
        bR = read(fd_device_fifo, &msg, sizeof(Message));
        if (bR == -1) {
            ErrExit("<device> read fifo failed");
        } else if (bR == sizeof(Message)) {
            for (; i < sizeof(messages_buffer); i++) {
                if (!messages_buffer[i]) {
                    messages_buffer[i] = &msg;
                    break;
                }
            }
            printf("<device %d> Read:\n", getpid());
            printDebugMessage(&msg);
        }
    } while (bR > 0);
    // TODO: salvare i messaggi nella lista di ack
}

void execDevice(int _id_device, int semid, int board_shmid, const char *path_to_position_file) 
{
    id_device = _id_device;
    // printf("<device pid: %d, id: %d> Created !!!\n", getpid(), _id_device);

    changeDeviceSignalHandler();

    board_shm_ptr = (pid_t *)getSharedMemory(board_shmid, 0);

    sprintf(path_to_device_fifo, "%s%d", base_path_to_device_fifo, getpid());

    // printf("<device pid: %d, id: %d> Creating fifo...\n", getpid(), id_device);
    int res_device_fifo = mkfifo(path_to_device_fifo, S_IRUSR | S_IWUSR);
    if (res_device_fifo == -1)
        ErrExit("<device> mkfifo failed");

    fd_device_fifo = open(path_to_device_fifo, O_RDONLY | O_NONBLOCK);
    if (fd_device_fifo == -1)
        ErrExit("<device> open fifo failed");

    position_fd = open(path_to_position_file, O_RDONLY);
    if (position_fd == -1)
        ErrExit("<device> open position file failed");

    char next_line[ROW_BUFFER_SIZE];

    while(1) {
        waitTurnAndBoard(semid);
        sendMessages();
        readMessages();

        int old_position_index = next_position.row * BOARD_COLS + next_position.col;

        readNextLine(next_line);
        getNextPosition(next_line, &next_position);

        int next_position_index = next_position.row * BOARD_COLS + next_position.col;

        // se la nuova posizione non è occupata, sposta il device
        if (board_shm_ptr[next_position_index] == 0) {
            // prima libera la precedente zona occupata
            board_shm_ptr[old_position_index] = 0;
            // occupa la nuova zona
            board_shm_ptr[next_position_index] = getpid();
        } else {
            // altrimenti il device resta fermo, e ripristina la posizione
            next_position.row = old_position_index / BOARD_COLS;
            next_position.col = old_position_index % BOARD_COLS;
        }
        printf("%d %d %d msgs: \n", getpid(), next_position.row, next_position.col);
        if (id_device == N_DEVICES - 1)
            printf("####################################################\n");

        signalEndTurn(semid);
    }
}