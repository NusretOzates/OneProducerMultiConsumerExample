cmake_minimum_required(VERSION 3.16)
project(SharedMemoryExampleTwo C)

set(CMAKE_C_STANDARD 11)

add_executable(SharedMemoryExampleTwo producer.c pc_protocol.h consumer.c producer2.c consumer2.c)
