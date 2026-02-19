#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "scheduler.h"
#include "pcap_reader.h"

#define BURST_SIZE 32

int main(int argc, char** argv) {
    
    std::cout << "[System] Initializing Packet Capture Engine..." << std::endl;
    PcapReader reader;
    
    std::string pcap_file = "traffic_large.pcap"; 
    if (argc > 1) {
        pcap_file = argv[1];
    }

    if (!reader.openTrace(pcap_file)) {
        std::cerr << "[Error] Failed to open pcap file: " << pcap_file << std::endl;
        return -1;
    }

    std::cout << "[System] Initializing Adaptive Scheduler..." << std::endl;
    Scheduler scheduler;
    scheduler.init();

    std::cout << "[System] Starting Benchmark on " << pcap_file << "..." << std::endl;
    
    uint64_t total_packets = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        std::vector<Packet> batch = reader.nextBatch(BURST_SIZE);

        if (batch.empty()) {
            break;
        }

        for (const auto& pkt : batch) {
            char* payload = (char*)pkt.payload.data();
            int payload_len = pkt.payload.size();

            scheduler.dispatch(payload, payload_len);
        }

        total_packets += batch.size();
    }
    
    reader.close();
    scheduler.flush_batch();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    double seconds = elapsed.count();
    double mpps = 0.0;
    if (seconds > 0) {
        mpps = (total_packets / 1000000.0) / seconds;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "          BENCHMARK RESULTS             " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << " Total Packets : " << total_packets << std::endl;
    std::cout << " CPU Processed : " << scheduler.get_cpu_packets() << std::endl;
    std::cout << " GPU Processed : " << scheduler.get_gpu_packets() << std::endl;
    std::cout << " Total Time    : " << std::fixed << std::setprecision(4) << seconds << " seconds" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << " AVERAGE SPEED : " << std::fixed << std::setprecision(3) << mpps << " Mpps" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}