#pragma once

typedef struct {
    int row;
    int col;
} Position;

void execDevice(int _id_device, int semid, pid_t *_board_shm_ptr, const char *path_to_position_file);
void freeDeviceResources();