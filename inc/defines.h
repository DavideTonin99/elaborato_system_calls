/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h>

// --numero di device
#define N_DEVICES 5

// --board settings
#define BOARD_ROWS 10
#define BOARD_COLS 10

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

// -- ACK LIST
/**
 * compareAcks: funzione usata nel qsort per ordinare gli ack prima della risposta
*/
int compareAcks (const void *a, const void *b);

/**
 * checkAckAvailable: controlla nella zona di memoria condivisa se c'è posto per aggiungere un ack
 * @param shm_ptr_acklist: puntatore alla zona di memoria condivisa della lista di ack
 * @return: 1 se c'è posto, 0 altrimenti
*/
int checkAckAvailable(Acknowledgment *shm_ptr_acklist);

/**
 * addAck: aggiunge l'ack di un messaggio alla lista degli ack
 * @param shm_ptr_acklist: puntatore alla zona di memoria condivisa della lista di ack
 * @param msg: messaggio di cui aggiungere l'ack
*/
void addAck(Acknowledgment *shm_ptr_acklist, Message *msg);

/**
 * ackListContains: controlla se la lista di ack contiene un certo ack (dati pid ricevente e message id)
 * @param shm_ptr_acklist: puntatore alla zona di memoria condivisa della lista di ack
 * @param pid_receiver: pid device ricevente
 * @param message_id: id messaggio
 * @return: 1 se la lista contiene l'ack, 0 altrimenti
*/
int ackListContains(Acknowledgment *shm_ptr_acklist, pid_t pid_receiver, int message_id);

/**
 * contAckByMessageId: conta gli ack nella lista di acknowledgments, dato un message id
 * @param shm_ptr_acklist: puntatore alla zona di memoria della ack list
 * @param message_id: id del messaggio
 * @return: il numero di ack (0 se non ce ne sono)
*/
int contAckByMessageId(Acknowledgment *shm_ptr_acklist, int message_id);
