/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h>

// -- numero di device
#define N_DEVICES 5

// -- board settings
#define BOARD_ROWS 5
#define BOARD_COLS 5

// --grandezza lista acknowledgemnt
#define SIZE_ACK_LIST 100

// --semafori
#define SEMNUM_BOARD N_DEVICES
#define SEMNUM_ACKLIST N_DEVICES+1

typedef struct
{
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    char message[256];
    double max_distance;
} Message;

typedef struct
{
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    time_t timestamp;
} Acknowledgment;

// contiene la lista di Acknowledgment
typedef struct
{
    long mtype;
    Acknowledgment ack[N_DEVICES];
} Response;

/**
 * printDebugMessage: funzione di DEBUG, stampa il messaggio
 * @param msg: messaggio
 */
void printDebugMessage(Message *msg);

/**
 * writeOutAck: scrive su file la lista di ack
 * @param msg: messaggio inviato dal client
 * @param response: la risposta con la lista di ack da parte del server
 */
void writeOutAck(Message *msg, Response *response);

int contAckByMessageId(Acknowledgment *shm_ptr_acklist, int message_id);