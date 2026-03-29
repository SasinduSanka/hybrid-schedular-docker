#include <cuda_runtime.h>
#include <iostream>
#include "gpu_worker.h"

// Each block processes one packet.
// Threads within the block cooperatively scan the payload in parallel to maximize throughput.
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

// Allocate persistent VRAM upfront to avoid the massive latency overhead
// of calling cudaMalloc on every single batch.
GPUWorker::GPUWorker(int batch_capacity, size_t max_bytes_per_batch) {
    this->max_batch_packets = batch_capacity;
    this->max_total_bytes = max_bytes_per_batch;
    this->current_stream_idx = 0;

    cudaMalloc((void**)&d_flat_buffer, max_total_bytes * NUM_STREAMS);
    cudaMalloc((void**)&d_offsets, max_batch_packets * NUM_STREAMS * sizeof(int));
    cudaMalloc((void**)&d_lengths, max_batch_packets * NUM_STREAMS * sizeof(int));
    cudaMalloc((void**)&d_results, max_batch_packets * NUM_STREAMS * sizeof(int));

    for (int i = 0; i < NUM_STREAMS; ++i) {
        cudaStreamCreate(&streams[i]);
    }

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "CUDA Error in initialization: " << cudaGetErrorString(err) << std::endl;
    }
}

// Queue async memory transfers and kernel execution on the current stream.
// This allows the CPU to immediately return to parsing packets while the GPU works.
void GPUWorker::launch_gpu_batch_async(
    const char* host_flat_buffer,
    const int* host_offsets,
    const int* host_lengths,
    int num_packets,
    int* host_results)
{
    if (num_packets == 0) return;

    int total_bytes = host_offsets[num_packets - 1] + host_lengths[num_packets - 1];

    if (total_bytes > max_total_bytes || num_packets > max_batch_packets) {
        std::cerr << "Error: Batch exceeds pre-allocated GPU memory!" << std::endl;
        return;
    }

    int stream_id = current_stream_idx;

    // Calculate the memory offsets for the active double-buffer stream
    size_t byte_offset = stream_id * max_total_bytes;
    int packet_offset = stream_id * max_batch_packets;

    cudaMemcpyAsync(d_flat_buffer + byte_offset, host_flat_buffer, total_bytes, cudaMemcpyHostToDevice, streams[stream_id]);
    cudaMemcpyAsync(d_offsets + packet_offset, host_offsets, num_packets * sizeof(int), cudaMemcpyHostToDevice, streams[stream_id]);
    cudaMemcpyAsync(d_lengths + packet_offset, host_lengths, num_packets * sizeof(int), cudaMemcpyHostToDevice, streams[stream_id]);

    cudaMemsetAsync(d_results + packet_offset, 0, num_packets * sizeof(int), streams[stream_id]);

    batch_scan_kernel<<<num_packets, 256, 0, streams[stream_id]>>>(
        d_flat_buffer + byte_offset,
        d_offsets + packet_offset,
        d_lengths + packet_offset,
        d_results + packet_offset
    );

    cudaMemcpyAsync(host_results, d_results + packet_offset, num_packets * sizeof(int), cudaMemcpyDeviceToHost, streams[stream_id]);

    // Toggle to the next stream for the subsequent batch
    current_stream_idx = (current_stream_idx + 1) % NUM_STREAMS;
}

// Block the host thread until all pending async operations across all streams are complete.
void GPUWorker::synchronize_all() {
    cudaDeviceSynchronize();
}

// Free persistent VRAM and destroy streams on shutdown.
GPUWorker::~GPUWorker() {
    cudaFree(d_flat_buffer);
    cudaFree(d_offsets);
    cudaFree(d_lengths);
    cudaFree(d_results);

    for (int i = 0; i < NUM_STREAMS; ++i) {
        cudaStreamDestroy(streams[i]);
    }
}