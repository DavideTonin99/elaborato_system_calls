/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "err_exit.h"
#include "fifo.h"
#include "defines.h"

#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"

void createFIFO(const char *path, mode_t mode) 
{
    if (mkfifo(path, mode) == -1)
        ErrExit("mkfifo failed");
}

int openFIFO(const char *path, mode_t mode) 
{
    int fd = open(path, mode);
    if (fd == -1)
        ErrExit("open fifo failed");
    return fd;
}

void writeFIFO(int fd, const Message *msg)
{
    int res = write(fd, msg, sizeof(Message));
    if (res == -1 || res != sizeof(Message))
        ErrExit("write message to fifo failed");
}

void closeFIFO(int fd)
{
    if (close(fd) == -1)
        ErrExit("close fifo failed");
}