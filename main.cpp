#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include "scheduler.h"

#define RX_RING_SIZE 1024
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

int main(int argc, char** argv) {
    std::vector<char*> eal_args;
    eal_args.push_back(argv[0]);
    eal_args.push_back((char*)"--no-shconf");
    eal_args.push_back((char*)"-l"); eal_args.push_back((char*)"0");
    eal_args.push_back((char*)"--log-level"); eal_args.push_back((char*)"lib.eal:error");
    eal_args.push_back((char*)"--iova-mode=pa");
    eal_args.push_back((char*)"--vdev=net_pcap0,rx_pcap=traffic_large.pcap");

    if (rte_eal_init(eal_args.size(), eal_args.data()) < 0) 
        rte_exit(EXIT_FAILURE, "EAL Init Failed\n");

    uint16_t port_id = 0;
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", 40000,
        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    struct rte_eth_conf port_conf = {};
    rte_eth_dev_configure(port_id, 1, 1, &port_conf);
    rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
    rte_eth_dev_start(port_id);

    std::cout << "[System] Initializing Adaptive Scheduler..." << std::endl;
    Scheduler scheduler;
    scheduler.init();

    std::cout << "[System] Starting Benchmark..." << std::endl;
    struct rte_mbuf *bufs[BURST_SIZE];
    uint64_t total_packets = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    int empty_reads = 0;

    while (true) {
        const uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, BURST_SIZE);

        if (nb_rx > 0) {
            empty_reads = 0;
            for (int i = 0; i < nb_rx; i++) {
                char* payload = rte_pktmbuf_mtod(bufs[i], char*);
                int payload_len = rte_pktmbuf_data_len(bufs[i]);
                scheduler.dispatch(payload, payload_len);
                rte_pktmbuf_free(bufs[i]);
            }
            total_packets += nb_rx;
        } else {
            empty_reads++;
            if (empty_reads > 10000) { 
                break; 
            }
        }
    }
    
    scheduler.flush_batch();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    double seconds = elapsed.count();
    double mpps = (total_packets / 1000000.0) / seconds;

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