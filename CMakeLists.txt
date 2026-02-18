cmake_minimum_required(VERSION 3.10)
project(HybridPacketInspector LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Dependencies
find_package(PkgConfig REQUIRED)

# Libpcap
pkg_check_modules(PCAP REQUIRED libpcap)

# Hyperscan
find_path(HYPERSCAN_INCLUDE_DIR hs/hs.h)
find_library(HYPERSCAN_LIBRARY NAMES hs hyperscan)
if(NOT HYPERSCAN_LIBRARY)
    message(FATAL_ERROR "Hyperscan library not found! Ensure libhyperscan-dev is installed.")
endif()

# CUDA
find_package(CUDAToolkit REQUIRED)
set(CMAKE_CUDA_ARCHITECTURES 86) 

# Executables (ADDED cpu_worker.cpp HERE)
add_executable(inspector 
    main.cpp 
    pcap_reader.cpp 
    scheduler.cpp 
    cpu_worker.cpp
    gpu_worker.cu 
)

# Link Libraries
target_link_libraries(inspector
    PRIVATE
    ${PCAP_LIBRARIES}
    ${HYPERSCAN_LIBRARY}
    CUDA::cudart
    pthread
)

# Include Directories
target_include_directories(inspector PUBLIC
    ${PCAP_INCLUDE_DIRS}
    ${HYPERSCAN_INCLUDE_DIR}
)
