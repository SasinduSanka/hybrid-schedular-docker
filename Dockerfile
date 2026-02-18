FROM nvidia/cuda:12.2.0-devel-ubuntu22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    python3 \
    vim \
    libpcap-dev \
    libhyperscan-dev \
    && rm -rf /var/lib/apt/lists/*
    
WORKDIR /app

COPY . /app

RUN rm -rf build CMakeCache.txt CMakeFiles && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

CMD ["/bin/bash"]