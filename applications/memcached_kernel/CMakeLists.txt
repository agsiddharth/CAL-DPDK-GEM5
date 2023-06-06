cmake_minimum_required (VERSION 3.16)

# gcc setup
add_definitions(-std=c++20 -O3 -march=native)
add_definitions(-Wall -Wextra -Wabi -Wsign-conversion -Wformat -Wformat-security)
add_definitions(-pedantic)
add_definitions(-fstack-protector -fPIE -fPIC)
add_definitions(-Wno-unused-function)
add_definitions(-D_FORTIFY_SOURCE=2)

# Common third-party modules
find_package(gflags REQUIRED)

#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#
add_executable(memcached_client memcached_client.cc)
target_link_libraries(memcached_client gflags)
