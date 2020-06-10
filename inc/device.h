#pragma once

#define MSG_BUFFER_SIZE 10
#define ROW_BUFFER_SIZE 50

// contiene la posizione del device
typedef struct {
    int row;
    int col;
} Position;

// -- LIBERAZIONE RISORSE, SIGNAL HANDLER PER TERMINAZIONE DEVICE
/**
 * 
*/
void freeDeviceResources();

/**
 * 
*/
void deviceSigHandler(int sig);

/**
 * 
*/
void changeDeviceSignalHandler();

// -- ESECUZIONE DEVICE
/**
 * @param _id_device
 * @param semid
 * @param shmid_board
 * @param shmid_acklist
 * @param path_to_position_file
*/
void execDevice(int _id_device, int semid, int shmid_board, int shmid_acklist, const char *path_to_position_file);

// -- SINCRONIZZAZIONE SEMAFORI
/**
 * @param semid
*/
void waitTurnAndBoard(int semid);

/**
 * @param semid
*/
void signalEndTurn(int semid);

// -- POSIZIONE DEVICE
/**
 * @param next_line: puntatore alla prossima riga letta dal file
*/
void readNextLine(char *next_line);

/**
 * @param next_line: putatore alla riga letta dal file posizioni
 * @param position: puntatore alla posizione del device
*/
void getNextPosition(char *next_line, Position *position);

// -- LETTURA MESSAGGI
/**
 * @param messages_buffer
 * @param n_messages
 * @param semid
*/
void readMessages(Message *messages_buffer, int *n_messages, int semid);

// -- OPERAZIONI SU ACKNOWLEDGMENT LIST
/**
 * 
*/
int checkAckAvailable();

/**
 * @param msg: messaggio di cui aggiungere l'ack
*/
void addAck(Message *msg);

/**
 * @param pid_receiver: pid device ricevente
 * @param message_id
*/
int ackListContains(pid_t pid_receiver, int message_id);

// -- INVIO MESSAGGI
/**
 * @param position
 * @param message_id
 * @param max_distance
*/
pid_t searchAvailableDevice(Position *position, int message_id, double max_distance);

/**
 * @param position
 * @param messages_buffer
 * @param n_messages
 * @param semid
*/
void sendMessages(Position *position, Message *messages_buffer, int *n_messages, int semid);