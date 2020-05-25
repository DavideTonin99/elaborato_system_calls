/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "stdio.h"
#include "stdlib.h"

#include "defines.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s msg_queue_key\n", argv[0]);
        return 0;
    }
    return 0;
}