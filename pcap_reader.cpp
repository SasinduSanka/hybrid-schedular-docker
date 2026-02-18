#include "pcap_reader.h"
#include <iostream>

PcapReader::PcapReader() : handle(nullptr) {}

PcapReader::~PcapReader() {
    close();
}

bool PcapReader::openTrace(const std::string& filename) {
    handle = pcap_open_offline(filename.c_str(), errbuf);
    if (handle == nullptr) {
        std::cerr << "Error opening pcap: " << errbuf << std::endl;
        return false;
    }
    return true;
}

void PcapReader::close() {
    if (handle != nullptr) {
        pcap_close(handle);
        handle = nullptr;
    }
}

std::vector<Packet> PcapReader::nextBatch(int batchSize) {
    std::vector<Packet> batch;
    struct pcap_pkthdr* header;
    const u_char* data;
    
    int count = 0;

    while (count < batchSize) {
        int res = pcap_next_ex(handle, &header, &data);
        
        if (res == 0) continue;
        if (res == -1 || res == -2) break;

        Packet pkt;
        pkt.payload.assign(data, data + header->caplen);
        batch.push_back(pkt);
        
        count++;
    }

    return batch;
}