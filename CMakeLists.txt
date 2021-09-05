cmake_minimum_required(VERSION 3.0)
project(biu)

set(CMAKE_CXX_STANDARD 14)

add_executable(biu main.cpp msg_handler.cpp)
target_link_libraries(biu -lpthread)