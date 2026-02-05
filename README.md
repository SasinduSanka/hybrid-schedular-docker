# Hybrid CPU/GPU Packet Scheduler
**Student:** Sasindu Opatha (ID: 20210979)

## Prerequisites
- Docker Engine
- NVIDIA Drivers (for GPU offloading)
- Linux Host (Recommended for Hugepages)

## How to Run
1. Allocate Hugepages (Required for DPDK):
   sudo bash -c "echo 512 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages"

2. Build and Run via Docker:
   docker compose run --rm inspector

3. Run the Benchmark (Inside the container):
   ./inspector

## Project Structure
- /src        : Source code (main.cpp, scheduler.cpp, gpu_worker.cu)
- Dockerfile  : Environment definition
- Makefile    : Build instructions