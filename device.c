#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/stat.h"

#include "defines.h"
#include "err_exit.h"
#include "device.h"

int id_device; // numero del device nell'intervallo [1,N_DEVICES]

const char *base_path_to_device_fifo = "/tmp/dev_fifo.";
char path_to_device_fifo[25];
int fd_device_fifo;

void removeFIFO() {
    if(unlink(path_to_device_fifo) == -1)
        ErrExitDevice("unlink fifo failed");
    
    if(close(fd_device_fifo) == -1)
        ErrExitDevice("close fifo failed");
}

void deviceSigHandler(int sig)
{
    if (sig == SIGTERM) {
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
        ErrExitDevice("sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExitDevice("sigprocmask failed");

    if (signal(SIGTERM, deviceSigHandler) == SIG_ERR)
        ErrExitDevice("change signal handler failed");
}

void execDevice(int _id_device, int semid, pid_t *board_shm_ptr) {
    id_device = _id_device;

    changeDeviceSignalHandler();

    sprintf(path_to_device_fifo, "%s%d", base_path_to_device_fifo, getpid());

    int res_device_fifo = mkfifo(path_to_device_fifo, S_IRUSR | S_IWUSR);
    if (res_device_fifo == -1)
        ErrExitDevice("mkfifo failed");

    fd_device_fifo = open(path_to_device_fifo, O_RDONLY);
    if (fd_device_fifo == -1)
        ErrExitDevice("open fifo failed");

    if (close(fd_device_fifo) == -1) {
        ErrExitDevice("close fifo failed");
    }
}

void ErrExitDevice(const char *msg) {
    printf("<device pid: %d, id: %d> ", getpid(), id_device);
    ErrExit(msg);
}