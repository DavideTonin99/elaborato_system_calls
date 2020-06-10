/// @file server.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del server
#pragma once

#include "defines.h"

/**
 * freeResources: libera le risorse utilizzate dal server
 * Rimuove il set di semafori e le zone di memoria condivisa
*/
void freeResources();

/**
 * sigHandler: handler dei segnali per il server
 * Gestisce solo il segnale SIGTERM, liberando le risorse e terminando i processi figli, e poi se stesso
 * @param sig: segnale in ingresso
 * 
*/
void sigHandler(int sig);

/**
 * changeSignalHandler: cambia il signal handler del server, 
 * per gestire solo il segnale SIGTERM
*/
void changeSignalHandler();

/**
 * initDevices: avvia i processi figli (DEVICE)
 * @param n_devices: numero di device da avviare
 * @param path_to_position_file: path del file posizioni
*/
void initDevices(int n_devices, const char *path_to_position_file);

/**
 * initAckManager: avvia il processo figlio ACK MANAGER
*/
void initAckManager();