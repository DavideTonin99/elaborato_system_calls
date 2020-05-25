/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        return 0;
    }

    return 0;
}