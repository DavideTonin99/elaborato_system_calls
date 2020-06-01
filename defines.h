/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h>

#define N_DEVICES 5
#define RESPONSE_MTYPE 1

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