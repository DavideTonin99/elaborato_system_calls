/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once
#include "sys/types.h"
#include "defines.h"

/**
 * 
*/
void createFIFO(const char *path, mode_t mode);

/**
 * 
*/
int openFIFO(const char *path, mode_t mode);

/**
 * 
*/
void writeFIFO(int fd, const Message *msg);

/**
 * 
*/
void closeFIFO(int fd);