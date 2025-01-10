cmake_minimum_required(VERSION 3.29)
project(Projektna)

# Add executable
add_executable(Projektna main.cpp)

# Find MPI and Link it
find_package(MPI REQUIRED)
target_link_libraries(Projektna PUBLIC MPI::MPI_CXX)

# Set the OpenSSL root directory (adjust this path if necessary)
set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")  # Change to your MSYS2 installation folder

# Find OpenSSL and Link it
find_package(OpenSSL REQUIRED)
target_link_libraries(Projektna PRIVATE OpenSSL::SSL OpenSSL::Crypto)