/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once
#include "stdlib.h"
#include "sys/types.h"

/**
 * allocSharedMemory: crea, se non esiste, un segmento di memoria condivisa di size bytes e chiave shm_key
 * @param shm_key: chiave del segmento di memoria condivisa
 * @param size: grandezza del segmento di memoria
 * @return: shmid se ha successo, altrimenti termina il processo chiamante
 */
int allocSharedMemory(key_t shm_key, size_t size);

/**
 * getSharedMemory: attacca il segmento di memoria condivisa allo spazio virtuale di indirizzamento del processo chiamante
 * @param shmid: id del segmento di memoria condivisa
 * @param shmflg: flags
 * @return: puntatore all'indirizzo al quale la memoria condivisa è stata attaccata con successo,
 * altrimenti termina il processo chiamante
 */
void *getSharedMemory(int shmid, int shmflg);

/**
 * freeSharedMemory: DETACH il segmento di memoria condivisa dallo spazio virtuale di indirizzamento del processo chiamante
 * @param ptr_sh: puntatore al segmento di memoria
 * Se non ha successo, termina il processo chiamante
 */
void freeSharedMemory(void *ptr_sh);

/**
 * removeSharedMemory: rimuove un segmento di memoria condivisa
 * @param shmid: id segmento di memoria
 * Se non ha successo, termina il processo chiamante
 */
void removeSharedMemory(int shmid);
