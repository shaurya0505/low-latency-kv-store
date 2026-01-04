cmake_minimum_required(VERSION 3.10)
project(LowLatencyKVStore VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library
add_library(kvstore STATIC
    src/kv_store.cpp
)

# Example executable
add_executable(kv_example
    examples/example.cpp
)
target_link_libraries(kv_example kvstore pthread)

# Tests
enable_testing()
add_executable(kv_tests
    tests/test_kv_store.cpp
)
target_link_libraries(kv_tests kvstore pthread)

add_test(NAME KVStoreTests COMMAND kv_tests)

# Installation
install(TARGETS kvstore DESTINATION lib)
install(FILES include/kv_store.hpp DESTINATION include)
