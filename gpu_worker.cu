#include <cuda_runtime.h>
#include <iostream>
#include "gpu_worker.h"

__global__ void batch_scan_kernel(char* all_data, int* offsets, int* lengths, int* results) {
    int packet_idx = blockIdx.x; 
    
    int my_start = offsets[packet_idx];
    int my_len = lengths[packet_idx];
    
    char* my_packet = &all_data[my_start];

    for (int i = threadIdx.x; i < my_len; i += blockDim.x) {
        if (my_packet[i] == 'G') {
            results[packet_idx] = 1;
        }
    }
}

extern "C" void launch_gpu_batch(char* h_buffer, int* h_offsets, int* h_lengths, int num_packets) {
    if (num_packets == 0) return;

    int total_bytes = h_offsets[num_packets - 1] + h_lengths[num_packets - 1];

    char* d_buffer;
    int *d_offsets, *d_lengths, *d_results;
    int* h_results = new int[num_packets];

    cudaMalloc((void**)&d_buffer, total_bytes);
    cudaMalloc((void**)&d_offsets, num_packets * sizeof(int));
    cudaMalloc((void**)&d_lengths, num_packets * sizeof(int));
    cudaMalloc((void**)&d_results, num_packets * sizeof(int));

    cudaMemcpy(d_buffer, h_buffer, total_bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_offsets, h_offsets, num_packets * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_lengths, h_lengths, num_packets * sizeof(int), cudaMemcpyHostToDevice);
    
    cudaMemset(d_results, 0, num_packets * sizeof(int));

    batch_scan_kernel<<<num_packets, 256>>>(d_buffer, d_offsets, d_lengths, d_results);
    
    cudaDeviceSynchronize();

    cudaMemcpy(h_results, d_results, num_packets * sizeof(int), cudaMemcpyDeviceToHost);

    for (int i = 0; i < num_packets; i++) {
        if (h_results[i] == 1) {
            // std::cout << "[GPU] Alert in packet #" << i << " of batch!" << std::endl;
        }
    }

    cudaFree(d_buffer);
    cudaFree(d_offsets);
    cudaFree(d_lengths);
    cudaFree(d_results);
    delete[] h_results;
}