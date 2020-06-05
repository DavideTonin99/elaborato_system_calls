#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/stat.h"

#include "defines.h"
#include "err_exit.h"
#include "device.h"

int id_device; 

const char *base_path_to_device_fifo = "/tmp/dev_fifo.";
char path_to_device_fifo[25];
int position_fd;

void removeFIFO() {
    if(unlink(path_to_device_fifo) == -1)
        ErrExit("<device> unlink fifo failed");
    
    if(close(position_fd) == -1)
        ErrExit("<device> close fifo failed");
}

void deviceSigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT) {
        printf("<device pid: %d>remove resources and exit...\n", getpid());
        removeFIFO();
        exit(0);
    }
}

void changeDeviceSignalHandler() 
{
    printf("<device pid: %d, id: %d> Changing signal handler...\n", getpid(), id_device);
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

void execDevice(int _id_device, int semid, pid_t *board_shm_ptr) {
    printf("<device pid: %d, id: %d> Created !!!\n", getpid(), id_device);
    id_device = _id_device;

    changeDeviceSignalHandler();

    sprintf(path_to_device_fifo, "%s%d", base_path_to_device_fifo, getpid());

    printf("<device pid: %d, id: %d> Creating fifo...\n", getpid(), id_device);
    int res_device_fifo = mkfifo(path_to_device_fifo, S_IRUSR | S_IWUSR);
    if (res_device_fifo == -1)
        ErrExit("<device> mkfifo failed");

    int fd_device_fifo = open(path_to_device_fifo, O_RDONLY | O_NONBLOCK);
    if (fd_device_fifo == -1)
        ErrExit("<device> open fifo failed");

    if (close(fd_device_fifo) == -1) {
        ErrExit("<device> close fifo failed");
    }
    while(1) {
        
    }
}