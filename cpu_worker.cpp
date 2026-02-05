#include "cpu_worker.h"

CpuWorker::CpuWorker() {
    database = nullptr;
    scratch = nullptr;
}

CpuWorker::~CpuWorker() {
    if (scratch) hs_free_scratch(scratch);
    if (database) hs_free_database(database);
}

bool CpuWorker::init_pattern(const char* pattern) {
    hs_compile_error_t* compile_err;
    
    if (hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, nullptr, &database, &compile_err) != HS_SUCCESS) {
        std::cerr << "[CPU] ERROR: Failed to compile pattern: " << compile_err->message << std::endl;
        hs_free_compile_error(compile_err);
        return false;
    }

    if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
        std::cerr << "[CPU] ERROR: Failed to allocate scratch space." << std::endl;
        return false;
    }

    std::cout << "[CPU] Hyperscan Database Compiled Successfully." << std::endl;
    return true;
}

int CpuWorker::onMatch(unsigned int id, unsigned long long from, 
                       unsigned long long to, unsigned int flags, void* ctx) {
    bool* found = (bool*)ctx;
    *found = true;
    return 0;
}

bool CpuWorker::scan_packet(const char* data, int len) {
    bool found = false;

    if (hs_scan(database, data, len, 0, scratch, onMatch, &found) != HS_SUCCESS) {
        std::cerr << "[CPU] Hyperscan internal error." << std::endl;
    }

    return found;
}