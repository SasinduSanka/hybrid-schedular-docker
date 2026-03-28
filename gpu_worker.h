#ifndef GPU_WORKER_H
#define GPU_WORKER_H

#include <cuda_runtime.h>
#include <cstddef>

class GPUWorker {
private:
    // Pointers for persistent device memory allocated once during initialization.
    char* d_flat_buffer;
    int* d_offsets;
    int* d_lengths;
    int* d_results;

    // Execution streams to enable asynchronous double-buffering (H2D, D2H, and Kernel overlap).
    static const int NUM_STREAMS = 2;
    cudaStream_t streams[NUM_STREAMS];

    // Capacity limits enforced to prevent VRAM overflow.
    int max_batch_packets;
    size_t max_total_bytes;
    int current_stream_idx;

public:
    // Allocate persistent device memory and create streams.
    GPUWorker(int batch_capacity, size_t max_bytes_per_batch);

    // Free device allocations and destroy streams.
    ~GPUWorker();

    // Hot path: Dispatch the batch asynchronously using the current CUDA stream.
    // Avoids blocking the host thread.
    void launch_gpu_batch_async(
        const char* host_flat_buffer,
        const int* host_offsets,
        const int* host_lengths,
        int num_packets,
        int* host_results
    );

    // Block the host until all pending async operations in all streams are complete.
    void synchronize_all();
};

#endif