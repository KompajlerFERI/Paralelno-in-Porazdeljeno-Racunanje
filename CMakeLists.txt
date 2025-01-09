cmake_minimum_required(VERSION 3.29)
project(Projektna)

set(CMAKE_CXX_STANDARD 20)

# Add MPI Package to Project
find_package(MPI REQUIRED)

add_executable(Projektna main.cpp)

# Link MPI libraries
target_link_libraries(Projektna PRIVATE MPI::MPI_CXX)
