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

# Fetch and link cpp-httplib
FetchContent_Declare(httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.10.8.tar.gz)
FetchContent_MakeAvailable(httplib)
target_include_directories(Projektna PRIVATE ${httplib_SOURCE_DIR})

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

set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")
# Find OpenSSL and Link it
find_package(OpenSSL REQUIRED)
target_link_libraries(Projektna PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Link against ws2_32 library for Winsock functions
target_link_libraries(Projektna PRIVATE ws2_32)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -O2")