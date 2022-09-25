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
