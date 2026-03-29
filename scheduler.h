#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "cpu_worker.h"
#include "gpu_worker.h"
#include <iostream>
#include <vector>

class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    void init();
    void dispatch(char* packet_data, int packet_len);
    
    void flush_batch(); 

    uint64_t get_cpu_packets() { return cpu_packets; }
    uint64_t get_gpu_packets() { return gpu_packets; }

private:
    CpuWorker cpu_worker;
    uint64_t cpu_packets = 0;
    uint64_t gpu_packets = 0;
    
    const int GPU_THRESHOLD = 1024; 
    
    const size_t BATCH_SIZE = 32;
    
    std::vector<char> batch_flat_buffer;
    std::vector<int> batch_offsets;
    std::vector<int> batch_lengths;
    int current_batch_count = 0;
};

#endif
