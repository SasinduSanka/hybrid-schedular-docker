#include "terminal_dashboard.h"
#include <iostream>
#include <iomanip>

void TerminalDashboard::init() {
    // \033[2J clears the screen, \033[H moves the cursor to the top-left
    std::cout << "\033[2J\033[H"; 
    std::cout << "========================================================\n";
    std::cout << "  HYBRID CPU/GPU SCHEDULER - LIVE TELEMETRY             \n";
    std::cout << "========================================================\n\n";
}

void TerminalDashboard::update(uint64_t cpu_packets, uint64_t gpu_packets, double elapsed_time) {
    uint64_t total = cpu_packets + gpu_packets;
    
    // Prevent division by zero during the very first render tick
    double mpps = 0.0;
    if (elapsed_time > 0.0) {
        mpps = (total / elapsed_time) / 1000000.0;
    }

    // \033[5;0H moves the cursor to Line 5, Column 0 to overwrite previous data
    std::cout << "\033[5;0H"; 
    
    std::cout << "  Elapsed Time : " << std::fixed << std::setprecision(2) << elapsed_time << " s\n";
    // \033[1;32m turns the text bold green, \033[0m resets it
    std::cout << "  Current Speed: " << "\033[1;32m" << mpps << " Mpps" << "\033[0m\n\n"; 

    std::cout << "  --- Routing Distribution ---\n";
    std::cout << "  CPU Handled  : " << std::setw(10) << cpu_packets << " (" << calc_percent(cpu_packets, total) << "%)\n";
    std::cout << "  GPU Handled  : " << std::setw(10) << gpu_packets << " (" << calc_percent(gpu_packets, total) << "%)\n";
    std::cout << "  Total Packets: " << std::setw(10) << total << "\n";
    
    // Force the terminal to draw immediately
    std::cout << std::flush; 
}

int TerminalDashboard::calc_percent(uint64_t part, uint64_t total) {
    if (total == 0) return 0;
    return (part * 100) / total;
}