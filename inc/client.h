/// @file client.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del client
#pragma once

#include "defines.h"

/**
 * readInt: legge un numero intero da un buffer di char.
 * Il numero deve essere maggiore di zero
 * @param buffer: buffer di caratteri
 * @return res: numero intero
*/
int readInt(const char *buffer);

/**
 * readDouble: legge un numero double da un buffer di char.
 * Il numero deve essere maggiore di zero
 * @param buffer: buffer di caratteri
 * @return res: numero double
*/
double readDouble(const char *buffer);

/**
 * sendMessage: invia il messaggio al device scelto
 * @param msg: messaggio da inviare
*/
void sendMessage(Message *msg);

/**
 * writeOutAck: scrive su file la lista di ack
 * @param msg: messaggio inviato dal client
 * @param response: la risposta con la lista di ack da parte del server
 */
void writeOutAck(Message *msg, Response response);