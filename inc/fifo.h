/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once
#include "sys/types.h"
#include "defines.h"

/**
 * @description: crea una fifo
 * @param path: path della fifo
 * @param mode: permessi
*/
void createFIFO(const char *path, mode_t mode);

/**
 * @description: apre una fifo
 * @param path: path della fifo
 * @param flags: modalità di apertura
*/
int openFIFO(const char *path, int flags);

/**
 * @description: scrive un messaggio su una fifo
 * @param fd: file descriptor della fifo
 * @param msg: messaggio
*/
void writeFIFO(int fd, const Message *msg);

/**
 * @description: chiude una fifo
 * @param fd
*/
void closeFIFO(int fd);