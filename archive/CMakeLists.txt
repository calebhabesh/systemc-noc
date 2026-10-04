# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(SystemC_NoC)

# Specify the path to the SystemC installation
set(SYSTEMC_HOME /usr/local/systemc-3.0.0/)

# Include directories for SystemC and the current source directory
include_directories(${SYSTEMC_HOME}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Link directories for SystemC libraries
link_directories(${SYSTEMC_HOME}/lib)

# Specify the source files
set(SOURCES
    arbiter.cpp
    buf_fifo.cpp
    crossbar.cpp
    router.cpp
    sink.cpp
    source.cpp
    main_noc.cpp
)

# Add the executable
add_executable(NoC.x ${SOURCES})

# Link the executable with the SystemC and math libraries
target_link_libraries(NoC.x systemc m)

