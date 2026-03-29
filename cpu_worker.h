#ifndef CPU_WORKER_H
#define CPU_WORKER_H

#include <hs/hs.h>
#include <cstdint>
#include <string>

class CPUWorker {
private:
    hs_database_t* database;
    hs_scratch_t* scratch;

    static int match_handler(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, void *context);

public:
    // Initialize with a specific regex rule
    CPUWorker(const char* pattern);
    ~CPUWorker();

    int scan_packet(const uint8_t* packet_data, unsigned int length);
};

#endif