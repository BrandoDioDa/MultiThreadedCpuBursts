cmake_minimum_required(VERSION 3.21)
project(assign5 C)

set(CMAKE_C_STANDARD 99)

add_executable(assign5 main.c dLinkedList.h dLinkedList.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(assign5 PRIVATE Threads::Threads)