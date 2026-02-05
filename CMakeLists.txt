cmake_minimum_required(VERSION 3.10)
project(HybridPacketInspector)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

find_path(HYPERSCAN_INCLUDE_DIR hs/hs.h)
find_library(HYPERSCAN_LIBRARY NAMES hs hyperscan)

if(NOT HYPERSCAN_LIBRARY)
    message(FATAL_ERROR "Hyperscan library not found! Ensure libhyperscan-dev is installed.")
endif()

enable_language(CUDA)
set(CMAKE_CUDA_ARCHITECTURES 75)
find_package(CUDAToolkit REQUIRED)

add_executable(inspector main.cpp gpu_worker.cu cpu_worker.cpp scheduler.cpp)

target_link_libraries(inspector
    PRIVATE
    ${DPDK_LIBRARIES}
    ${HYPERSCAN_LIBRARY}
    pcap
    CUDA::cudart 
)

target_include_directories(inspector PUBLIC
    ${DPDK_INCLUDE_DIRS}
    ${HYPERSCAN_INCLUDE_DIR}
)