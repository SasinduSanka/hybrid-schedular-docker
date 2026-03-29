#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <thread>
#include "scheduler.h"
#include "pcap_reader.h"
#include "terminal_dashboard.h"

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

    TerminalDashboard dashboard;
    dashboard.init();

    std::atomic<bool> is_processing{true};
    auto start_time = std::chrono::high_resolution_clock::now();

    std::thread telemetry_thread([&]() {
        while (is_processing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = current_time - start_time;

            dashboard.update(
                scheduler.get_cpu_packets(),
                scheduler.get_gpu_packets(),
                elapsed.count()
            );
        }
    });

    uint64_t total_packets = 0;
    start_time = std::chrono::high_resolution_clock::now();

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
    scheduler.finish();

    auto end_time = std::chrono::high_resolution_clock::now();

    // 2. Stop the telemetry thread safely
    is_processing = false;
    telemetry_thread.join();

    // 3. One final draw to ensure the dashboard shows the exact final numbers

    std::chrono::duration<double> final_elapsed = end_time - start_time;

    dashboard.update(
        scheduler.get_cpu_packets(),
        scheduler.get_gpu_packets(),
        final_elapsed.count()
    );

    // 4. Move the terminal cursor down so the shell prompt doesn't overwrite the UI
    std::cout << "\n\n\n\n\n\n";

    return 0;
}