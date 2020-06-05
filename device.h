#pragma once

void execDevice(int _id_device, int semid, pid_t *board_shm_ptr);
void removeFIFO();