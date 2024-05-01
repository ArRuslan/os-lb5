cmake_minimum_required(VERSION 3.28)
project(os_lb5)

set(CMAKE_CXX_STANDARD 17)

add_executable(os_lb5 main.cpp caches.hpp)
add_executable(os_lb5t23 task_23_45.cpp)
add_executable(os_lb5t45 task_23_45.cpp)

add_executable(os_lb5t7 lb4-mmap/lb4mailbox.cpp lb4-mmap/main.cpp)

target_link_options(os_lb5 PUBLIC "--entry=main")
target_compile_options(os_lb5 PUBLIC "-Wl,-emain")
