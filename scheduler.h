#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "cpu_worker.h"
#include "gpu_worker.h"
#include <cuda_runtime.h>
#include <iostream>
#include <cstdint>

class Scheduler {
public:
    Scheduler();
    ~Scheduler(); // destructor to clean up pinned memory

    void init();
    void dispatch(char* packet_data, int packet_len);

    void flush_batch();
    void finish();

    uint64_t get_cpu_packets() { return cpu_packets; }
    uint64_t get_gpu_packets() { return gpu_packets; }

private:
    CPUWorker* cpu_worker;
    GPUWorker* gpu_worker;

    uint64_t cpu_packets = 0;
    uint64_t gpu_packets = 0;

    // Tuned for GPU parallel throughput
    static const int MAX_BATCH_SIZE = 4096;
    // 10 MB per batch
    static const size_t MAX_BATCH_BYTES = 10 * 1024 * 1024;

    // Pinned Memory Pointers for Double Buffering
    char* host_flat_buffer;
    int* host_offsets;
    int* host_lengths;
    int* host_results;

    // State tracking for the current batch
    int current_batch_count = 0;
    size_t current_batch_bytes = 0;

    // Toggles between 0 and 1 for the streams
    int stream_toggle = 0;
};

#endif