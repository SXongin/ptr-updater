cmake_minimum_required(VERSION 3.16)

project(lock-free_swap)

add_executable(
  main
  main.cpp
)

target_link_libraries(
  main
  PRIVATE
    pthread
)

include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG d572f4777349d43653b21d6c2fc63020ab326db2 # v1.7.1
)
FetchContent_GetProperties(benchmark)
if(NOT benchmark_Populated)
  FetchContent_Populate(benchmark)
  set(BENCHMARK_ENABLE_TESTING OFF)
  add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
endif()
# FetchContent_MakeAvailable(benchmark)

add_executable(
  benchmark_ptr_updater
  benchmark_ptr_updater.cpp
)

target_link_libraries(
  benchmark_ptr_updater
  PRIVATE
    benchmark_main
)
