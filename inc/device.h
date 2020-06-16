#pragma once

#define MSG_BUFFER_SIZE 10
#define ROW_BUFFER_SIZE 200

// contiene la posizione del device
typedef struct {
    int row;
    int col;
} Position;

// -- LIBERAZIONE RISORSE, SIGNAL HANDLER PER TERMINAZIONE DEVICE
/**
 * freeDeviceResources: libera le risorse utilizzate dal device
 * Scollega le zone di memoria condivisa
 * Chiude la fifo e la cancella
 * Chiude il file delle posizioi
*/
void freeDeviceResources();

/**
 * deviceSigHandler: signal handler per il device.
 * Gestisce solo SIGTERM
 * @param sig: segnale in ingresso
*/
void deviceSigHandler(int sig);

/**
 * changeDeviceSignalHandler: cambia il signal handler del device,
 * per gestire solo il segnale SIGTERM
*/
void changeDeviceSignalHandler();

// -- SINCRONIZZAZIONE SEMAFORI
/**
 * waitTurnAndBoard: il device attende il proprio turno e l'accesso alla scacchiera
 * @param semid: id set semafori
*/
void waitTurnAndBoard(int semid);

/**
 * signalEndTurn: sblocca il device successivo
 * Se il device è l'ultimo, blocca l'accesso alla board
 * @param semid: id set semafori
*/
void signalEndTurn(int semid);

// -- LETTURA MESSAGGI
/**
 * readMessages: carica i messaggi dalla FIFO nel buffer di ricezione, se ce ne sono
 * @param messages_buffer: buffer di ricezione dei messaggi del device
 * @param n_messages: numero di messaggi contenuti fino'ora nel buffer
 * @param semid: id set semafori
*/
void readMessages(Message *messages_buffer, int *n_messages, int semid);

// -- INVIO MESSAGGI
/**
 * searchAvailableDevice: cerca un device disponibile a cui inviare il messaggio
 * @param position: posizione del device
 * @param message_id: id messaggio
 * @param max_distance: massima distanza a cui inviare il messaggio
*/
pid_t searchAvailableDevice(Position *position, int message_id, double max_distance);

/**
 * sendMessages: invia i messaggi
 * @param position: posizione del device
 * @param messages_buffer: buffer dei messaggi del device
 * @param n_messages: numero di messaggi contenuti nel buffer
 * @param semid: id set semafori
*/
void sendMessages(Position *position, Message *messages_buffer, int *n_messages, int semid);

// -- POSIZIONE DEVICE
/**
 * readNextLine: legge una riga dal file posizioni, e salva il valore in next_line
 * @param next_line: puntatore alla prossima riga letta dal file
*/
void readNextLine(char *next_line);

/**
 * getNextPosition: prende la posizione del device da next_line, e la salva in position
 * @param next_line: putatore alla riga letta dal file posizioni
 * @param position: puntatore alla posizione del device, in cui salvare il valore della posizione
*/
void getNextPosition(char *next_line, Position *position);

/**
 * moveDevice: effettua il movimento del device
 * @param position: la posizione corrente del device
 * @param next_line: puntatore in cui salvare la prossima riga del file posizioni
*/
void moveDevice(Position *position, char *next_line);

// -- ESECUZIONE DEVICE
/**
 * printDebugDevice: stampa la riga di debug del device
 * @param position: posizione del device
 * @param n_messages: numero di messaggi del device
 * @param messages_buffer: buffer di messaggi del device
*/
void printDebugDevice(Position position, int n_messages, Message *messages_buffer);

/**
 * execDevice: esegue il device
 * @param _id_device: id del device (indice processo figlio generato dal padre)
 * @param semid: id set semafori
 * @param shmid_board: id zona di memoria condivisa della scacchiera
 * @param shmid_acklist: id zona di memoria condivisa della ack list
 * @param shmid_deviceslist: id zona di memoria condivisa della lista di device
 * @param path_to_position_file: path del file posizioni
*/
void execDevice(int _id_device, int semid, int shmid_board, int shmid_acklist, int shmid_deviceslist, const char *path_to_position_file);
