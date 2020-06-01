/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "stdio.h"
#include "stdlib.h"
#include "signal.h"

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

void sigHandler(int sig)
{
    if (sig == SIGTERM) {
        // TODO: chiusura di tutti i meccanismi di comunicazione/sincronizzazione tra processi

        printf("Server close processes and exit...\n", getpid());

        // terminazione processo server e figli
        kill(-getpid(), sig);
        exit(0);
    }
}

void changeSignalHandler()
{
    sigset_t signals_set;
    if (sigfillset(&signals_set) == -1)
        ErrExit("sigfillset failed");

    // rimuove SIGTERM
    sigdelset(&signals_set, SIGTERM);

    // blocca tutti i segnali, tranne SIGTERM che è stato rimosso
    if (sigprocmask(SIG_SETMASK, &signals_set, NULL) == -1)
        ErrExit("sigprocmask failed");

    if (signal(SIGTERM, sigHandler) == SIG_ERR)
        ErrExit("change signal handler failed");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        return 0;
    }

    changeSignalHandler();

    return 0;
}