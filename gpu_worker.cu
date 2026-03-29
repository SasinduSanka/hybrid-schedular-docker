#include <cuda_runtime.h>
#include <iostream>
#include "gpu_worker.h"

// pre-allocated device memory pointers
static char* d_buffer = nullptr;
static int* d_offsets = nullptr;
static int* d_lengths = nullptr;
static int* d_results = nullptr;
static int* h_results = nullptr;

// maximum expected sizes - configurable via init_gpu
static int g_max_batch_size = 0;
static int g_max_buffer_size = 0;

__global__ void batch_scan_kernel(char* all_data, int* offsets, int* lengths, int* results) {
    int packet_idx = blockIdx.x; 
    
    int my_start = offsets[packet_idx];
    int my_len = lengths[packet_idx];
    
    char* my_packet = &all_data[my_start];

    // optimize - Check if result is already set to avoid unnecessary global memory writes
    for (int i = threadIdx.x; i < my_len; i += blockDim.x) {
        if (my_packet[i] == 'G') {
            if (results[packet_idx] == 0) {
                results[packet_idx] = 1;
            }
        }
    }
}

extern "C" void init_gpu(int max_batch_size, int max_buffer_size) {
    g_max_batch_size = max_batch_size;
    g_max_buffer_size = max_buffer_size;

    cudaMalloc((void**)&d_buffer, max_buffer_size);
    cudaMalloc((void**)&d_offsets, max_batch_size * sizeof(int));
    cudaMalloc((void**)&d_lengths, max_batch_size * sizeof(int));
    cudaMalloc((void**)&d_results, max_batch_size * sizeof(int));

    //pinned memory for faster transfers
    cudaMallocHost((void**)&h_results, max_batch_size * sizeof(int));
}

extern "C" void cleanup_gpu() {
    if (d_buffer) cudaFree(d_buffer);
    if (d_offsets) cudaFree(d_offsets);
    if (d_lengths) cudaFree(d_lengths);
    if (d_results) cudaFree(d_results);
    if (h_results) cudaFreeHost(h_results);
}

extern "C" void launch_gpu_batch(char* h_buffer, int* h_offsets, int* h_lengths, int num_packets) {
    if (num_packets == 0) return;

    int total_bytes = h_offsets[num_packets - 1] + h_lengths[num_packets - 1];

    if (num_packets > g_max_batch_size || total_bytes > g_max_buffer_size) {
        std::cerr << "[GPU] Batch exceeds pre-allocated memory limits!" << std::endl;
        return;
    }

    // Async memory transfers to overlap with other work (requires using a stream in a more complex setup, but good practice)
    cudaMemcpyAsync(d_buffer, h_buffer, total_bytes, cudaMemcpyHostToDevice);
    cudaMemcpyAsync(d_offsets, h_offsets, num_packets * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpyAsync(d_lengths, h_lengths, num_packets * sizeof(int), cudaMemcpyHostToDevice);
    
    cudaMemsetAsync(d_results, 0, num_packets * sizeof(int));

    batch_scan_kernel<<<num_packets, 256>>>(d_buffer, d_offsets, d_lengths, d_results);

    cudaMemcpyAsync(h_results, d_results, num_packets * sizeof(int), cudaMemcpyDeviceToHost);

    // Synchronize to ensure all async operations are complete before reading h_results
    cudaDeviceSynchronize();

    for (int i = 0; i < num_packets; i++) {
        if (h_results[i] == 1) {
            // std::cout << "[GPU] Alert in packet #" << i << " of batch!" << std::endl;
        }
    }
}
