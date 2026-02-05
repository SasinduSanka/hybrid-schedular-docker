#ifndef GPU_WORKER_H
#define GPU_WORKER_H

#include <vector>

// void launch_gpu_inspection(char* packet_data, int packet_len);

extern "C" void launch_gpu_batch(
    char* flat_buffer, 
    int* offsets, 
    int* lengths, 
    int num_packets
);

#endif