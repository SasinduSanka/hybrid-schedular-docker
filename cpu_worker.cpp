#include "cpu_worker.h"
#include <iostream>

// Compile the regex pattern and allocate scratch space once during startup to avoid per-packet overhead.
CPUWorker::CPUWorker(const char* pattern) : database(nullptr), scratch(nullptr) {
    hs_compile_error_t* compile_err;

    if (hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, nullptr, &database, &compile_err) != HS_SUCCESS) {
        std::cerr << "[CPU] ERROR: Failed to compile pattern: " << compile_err->message << std::endl;
        hs_free_compile_error(compile_err);
        return;
    }

    if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
        std::cerr << "[CPU] ERROR: Failed to allocate scratch space." << std::endl;
        hs_free_database(database);
        return;
    }

    std::cout << "[CPU] Hyperscan Database Compiled Successfully for pattern: '" << pattern << "'" << std::endl;
}

// Free Hyperscan allocations on shutdown.
CPUWorker::~CPUWorker() {
    if (scratch) hs_free_scratch(scratch);
    if (database) hs_free_database(database);
}

// Match callback triggered by hs_scan.
int CPUWorker::match_handler(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, void *context) {
    int* match_count = static_cast<int*>(context);
    (*match_count)++;

    // Return 0 to instruct Hyperscan to continue searching the rest of the packet for additional matches.
    return 0;
}

// Hot path: search the incoming packet using our pre-compiled DFA database.
int CPUWorker::scan_packet(const uint8_t* packet_data, unsigned int length) {
    int match_count = 0;

    if (!database || !scratch) return 0;

    if (hs_scan(database, reinterpret_cast<const char*>(packet_data), length, 0, scratch, match_handler, &match_count) != HS_SUCCESS) {
        std::cerr << "[CPU] Hyperscan internal error during scan." << std::endl;
    }

    return match_count;
}