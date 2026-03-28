# Hybrid CPU/GPU Packet Inspection with Adaptive Scheduling

A prototype adaptive Deep Packet Inspection (DPI) engine dynamically routing traffic between CPU and GPU resources.

# Description

Modern enterprise networks operate at speeds too high for static security rules. If an Intrusion Detection System (IDS) sends all traffic to a CPU, it drops packets when hit with massive payloads. If it sends everything to a GPU, the PCIe transfer overhead creates severe latency for tiny packets.

This project solves that bottleneck by acting as an intelligent middleware "Traffic Cop." It ingests network traffic, inspects the payload size in real-time, and dynamically routes latency-sensitive tiny packets to the CPU (powered by Intel Hyperscan) and massive payloads to the GPU (powered by NVIDIA CUDA) for parallel processing.

*Note: This repository represents Phase 1 & 2 of the project. It currently utilizes `libpcap` for offline trace reading to strictly isolate and measure the PCIe hardware bottleneck. The final production architecture will transition to DPDK for kernel-bypass packet capture.*

## Prerequisites

To build and run this containerized application, your host machine must have the following installed:

* **Docker Engine** (v20.10 or newer)
* **NVIDIA Display Drivers** compatible with your GPU
* **NVIDIA Container Toolkit** (required to pass the GPU hardware into the Docker container)
* A sample `.pcap` network trace file (A standard MAWI dataset trace is recommended for benchmarking)

## Installation

Since the entire build environment (including C++ compilers, CMake, Intel Hyperscan, and CUDA toolkits) is containerized via Docker, local dependency installation is minimal.

1. **Clone the repository**:
   ```bash git clone [https://github.com/your-username/hybrid-packet-inspection.git](https://github.com/your-username/hybrid-packet-inspection.git) cd hybrid-packet-inspection ```
2. **Prepare your testing data**:
   Ensure your sample `.pcap` files are located in the root of the cloned directory (or a designated `/data` folder) so they can be mounted into the container.


## Building the Project

The project is compiled directly inside a clean Ubuntu-based Docker container. This ensures that the Intel Hyperscan and CUDA libraries link correctly without polluting your local host machine.

Run the following command in the root directory of the project:
```
bash sudo docker build -t hybrid-engine .
```

## Usage

To execute the engine, you must run the Docker container with GPU passthrough enabled (`--gpus all`) and mount your local directory so the container can read your `.pcap` files.

Run the following command:

```
sudo docker run --rm --gpus all -v $(pwd):/app/data hybrid-engine ./build/inspector /app/data/benchmark_sample.pcap
```

* `-v $(pwd):/app/data`: Mounts your current working directory into the container at `/app/data`.
* `/app/data/benchmark_sample.pcap`: The path to the specific trace file you want to inspect. Replace `benchmark_sample.pcap` with your actual file name.