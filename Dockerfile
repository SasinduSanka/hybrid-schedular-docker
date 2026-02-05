FROM nvidia/cuda:12.2.0-devel-ubuntu22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    python3 \
    libnuma-dev \
    pciutils \
    vim \
    libhyperscan-dev \
    dpdk \
    dpdk-dev \
    libdpdk-dev \
    libpcap-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

CMD ["/bin/bash"]