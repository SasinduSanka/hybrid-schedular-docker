#ifndef GPU_WORKER_H
#define GPU_WORKER_H

#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

void init_gpu(int max_batch_size, int max_buffer_size);
void cleanup_gpu();

void launch_gpu_batch(
    char* flat_buffer, 
    int* offsets, 
    int* lengths, 
    int num_packets
);

#ifdef __cplusplus
}
#endif

#endif
