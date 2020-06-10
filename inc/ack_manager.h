/// @file ack_manager.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche dell'ack manager.

#pragma once

/**
 * ackManagerFreeResources: libera le risorse utilizzate dall'ack manager
 * Rimuove la message queue e fa il detach della memoria condivisa
*/
void ackManagerFreeResources();

/**
 * ackManagerSigHandler: handler dei segnali per l'ack manager
 * Gestisce solo il segnale SIGTERM, liberando le risorse e terminando il processo
 * @param sig: segnale in ingresso
 * 
*/
void ackManagerSigHandler(int sig);

/**
 * changeAckManagerSignalHandler: cambia il signal handler dell'ack manager,
 * per gestire solo il segnale SIGTERM
*/
void changeAckManagerSignalHandler();

/**
 * sendResponseToClient: invia la risposta con gli acknowledgments al client
 * @param message_id: id del messaggio di cui inviare gli acknowledgments
*/
void sendResponseToClient(int message_id);

/**
 * ackManagerRoutine: controlla se tutti i dispositivi hanno ricevuto un messaggio.
 * In caso positivo, invia la risposta al client
*/
void ackManagerRoutine();

/**
 * execAckManager: esegue l'ack manager
 * @param shmid_acklist: id della zona di memoria condivisa in cui si trova la lista di acknowledgments
 * @param msg_queue_key: chiave della message queue per comunicare con i client
 * @param semid: id del set di semafori per gestire l'accesso alla memoria condivisa
*/
void execAckManager(int shmid_acklist, int msg_queue_key, int semid);