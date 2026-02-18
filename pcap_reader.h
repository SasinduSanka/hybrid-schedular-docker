#ifndef PCAP_READER_H
#define PCAP_READER_H

#include <string>
#include <vector>
#include <pcap.h>

struct Packet {
    std::vector<uint8_t> payload;
};

class PcapReader {
private:
    pcap_t* handle;
    char errbuf[PCAP_ERRBUF_SIZE];

public:
    PcapReader();
    ~PcapReader();

    bool openTrace(const std::string& filename);

    std::vector<Packet> nextBatch(int batchSize);

    void close();
};

#endif