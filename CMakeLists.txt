cmake_minimum_required(VERSION 3.29)
project(Projektna)

include(FetchContent)

# Add executable
add_executable(Projektna main.cpp
        Block/Block.cpp Block/Block.h
        BlockChain/BlockChain.cpp
        BlockChain/BlockChain.h
        sha256/sha256.cpp
        sha256/sha256.h
        Rating/Rating.cpp
        Rating/Rating.h)

# Fetch and link nlohmann/json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(Projektna PRIVATE nlohmann_json::nlohmann_json)

# Fetch and link OpenMP
FetchContent_Declare(openmp URL https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.0/openmp-14.0.0.src.tar.xz)
FetchContent_MakeAvailable(openmp)
find_package(OpenMP REQUIRED)
target_link_libraries(Projektna PRIVATE OpenMP::OpenMP_CXX)

# Fetch and link MPI
FetchContent_Declare(mpi URL https://github.com/open-mpi/ompi/archive/refs/tags/v4.1.1.tar.gz)
FetchContent_MakeAvailable(mpi)
find_package(MPI REQUIRED)
target_link_libraries(Projektna PUBLIC MPI::MPI_CXX)

# Fetch and link OpenSSL
FetchContent_Declare(openssl URL https://www.openssl.org/source/openssl-1.1.1l.tar.gz)
FetchContent_MakeAvailable(openssl)
find_package(OpenSSL REQUIRED)
target_link_libraries(Projektna PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -O2")