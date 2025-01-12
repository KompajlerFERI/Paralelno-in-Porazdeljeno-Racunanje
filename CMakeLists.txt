cmake_minimum_required(VERSION 3.29)
project(Projektna)

# Add executable
add_executable(Projektna main.cpp
        Block/Block.cpp Block/Block.h
        BlockChain/BlockChain.cpp
        BlockChain/BlockChain.h
        sha256/sha256.cpp
        sha256/sha256.h)

# Find MPI and Link it
find_package(MPI REQUIRED)
target_link_libraries(Projektna PUBLIC MPI::MPI_CXX)

# Find OpenMP and Link it
find_package(OpenMP REQUIRED)
target_link_libraries(Projektna PRIVATE OpenMP::OpenMP_CXX)

# Set the OpenSSL root directory (adjust this path if necessary)
set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")

# Find OpenSSL and Link it
find_package(OpenSSL REQUIRED)
target_link_libraries(Projektna PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -O2")