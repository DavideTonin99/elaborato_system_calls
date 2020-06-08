#pragma once

#define MSG_BUFFER_SIZE 10
#define ROW_BUFFER_SIZE 50

typedef struct {
    int row;
    int col;
} Position;

void execDevice(int _id_device, int semid, int shmid_board, int shmid_acklist, const char *path_to_position_file);
void freeDeviceResources();