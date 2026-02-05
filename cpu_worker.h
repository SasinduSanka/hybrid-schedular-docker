#ifndef CPU_WORKER_H
#define CPU_WORKER_H

#include <hs/hs.h>
#include <iostream>
#include <string>

class CpuWorker {
public:
    CpuWorker();
    ~CpuWorker();

    bool init_pattern(const char* pattern);

    bool scan_packet(const char* data, int len);

private:
    hs_database_t* database = nullptr;
    hs_scratch_t* scratch = nullptr;

    static int onMatch(unsigned int id, unsigned long long from,
                       unsigned long long to, unsigned int flags, void* ctx);
};

#endif