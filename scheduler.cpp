#include "scheduler.h"

Scheduler::Scheduler() {
    batch_flat_buffer.reserve(1024 * 32 * 2); 
    batch_offsets.reserve(32);
    batch_lengths.reserve(32);
}

void Scheduler::init() {
    if (!cpu_worker.init_pattern(".")) {
        std::cerr << "[Scheduler] Failed to init CPU worker." << std::endl;
    }
}

void Scheduler::dispatch(char* packet_data, int packet_len) {
    
    if (packet_len >= 50) {
        
        int current_offset = batch_flat_buffer.size();
        
        batch_flat_buffer.insert(batch_flat_buffer.end(), packet_data, packet_data + packet_len);
        
        batch_offsets.push_back(current_offset);
        batch_lengths.push_back(packet_len);
        current_batch_count++;
        gpu_packets++;

        if (current_batch_count >= BATCH_SIZE) {
            flush_batch();
        }

    } else {
        cpu_worker.scan_packet(packet_data, packet_len);
        cpu_packets++;
    }
}

void Scheduler::flush_batch() {
    if (current_batch_count == 0) return;

    launch_gpu_batch(
        batch_flat_buffer.data(), 
        batch_offsets.data(), 
        batch_lengths.data(), 
        current_batch_count
    );

    batch_flat_buffer.clear();
    batch_offsets.clear();
    batch_lengths.clear();
    current_batch_count = 0;
}