#pragma once

typedef struct {
    int row;
    int col;
} Position;

void execDevice(int _id_device, int semid, int board_shmid, const char *path_to_position_file);
void freeDeviceResources();