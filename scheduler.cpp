#include "scheduler.h"
#include <cstring>

// Allocate pinned memory to allow zero-copy, high-speed DMA transfers to the GPU.
// Allocate 2x the max capacity to support double buffering across two async streams.
Scheduler::Scheduler() {
    cpu_worker = new CPUWorker("G");
    gpu_worker = new GPUWorker(MAX_BATCH_SIZE, MAX_BATCH_BYTES);

    cudaHostAlloc((void**)&host_flat_buffer, MAX_BATCH_BYTES * 2, cudaHostAllocDefault);
    cudaHostAlloc((void**)&host_offsets, MAX_BATCH_SIZE * 2 * sizeof(int), cudaHostAllocDefault);
    cudaHostAlloc((void**)&host_lengths, MAX_BATCH_SIZE * 2 * sizeof(int), cudaHostAllocDefault);
    cudaHostAlloc((void**)&host_results, MAX_BATCH_SIZE * 2 * sizeof(int), cudaHostAllocDefault);
}

// Safely free pinned host memory back to the OS.
Scheduler::~Scheduler() {
    cudaFreeHost(host_flat_buffer);
    cudaFreeHost(host_offsets);
    cudaFreeHost(host_lengths);
    cudaFreeHost(host_results);
    delete cpu_worker;
    delete gpu_worker;
}

void Scheduler::init() {
    std::cout << "[Scheduler] Initialized Adaptive CPU/GPU Traffic Cop." << std::endl;
}

// Core routing logic based on payload size.
void Scheduler::dispatch(char* packet_data, int packet_len) {
    if (packet_len >= 50) {

        // Force an early flush if the current packet exceeds our VRAM byte limit.
        if (current_batch_bytes + packet_len > MAX_BATCH_BYTES) {
            flush_batch();
        }

        // Calculate offsets to fill the idle stream's buffer
        // while the GPU actively reads from the other stream's buffer.
        size_t byte_offset = (stream_toggle * MAX_BATCH_BYTES) + current_batch_bytes;
        int pkt_offset = (stream_toggle * MAX_BATCH_SIZE) + current_batch_count;

        std::memcpy(host_flat_buffer + byte_offset, packet_data, packet_len);

        host_offsets[pkt_offset] = current_batch_bytes;
        host_lengths[pkt_offset] = packet_len;

        current_batch_bytes += packet_len;
        current_batch_count++;
        gpu_packets++;

        // Trigger async dispatch once the batch capacity is reached.
        if (current_batch_count >= MAX_BATCH_SIZE) {
            flush_batch();
        }

    } else {
        // Route tiny packets to the CPU to avoid PCIe transfer overhead.
        cpu_worker->scan_packet(reinterpret_cast<const uint8_t*>(packet_data), packet_len);
        cpu_packets++;
    }
}

// Dispatch the current batch to the GPU asynchronously and immediately toggle streams.
void Scheduler::flush_batch() {
    if (current_batch_count == 0) return;

    size_t base_byte_offset = stream_toggle * MAX_BATCH_BYTES;
    int base_pkt_offset = stream_toggle * MAX_BATCH_SIZE;

    gpu_worker->launch_gpu_batch_async(
        host_flat_buffer + base_byte_offset,
        host_offsets + base_pkt_offset,
        host_lengths + base_pkt_offset,
        current_batch_count,
        host_results + base_pkt_offset
    );

    // Reset batch state and toggle the active stream index.
    current_batch_count = 0;
    current_batch_bytes = 0;
    stream_toggle = (stream_toggle + 1) % 2;
}

// Process any leftover packets and block until all async GPU streams complete.
void Scheduler::finish() {
    flush_batch();
    gpu_worker->synchronize_all();
}