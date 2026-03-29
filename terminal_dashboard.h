#ifndef TERMINAL_DASHBOARD_H
#define TERMINAL_DASHBOARD_H

#include <cstdint>

class TerminalDashboard {
public:
    // Initializes the dashboard header and clears the screen
    void init();

    // Updates the live telemetry metrics on the screen without scrolling
    void update(uint64_t cpu_packets, uint64_t gpu_packets, double elapsed_time);

private:
    // Helper function to calculate the percentage split
    int calc_percent(uint64_t part, uint64_t total);
};

#endif