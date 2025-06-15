cmake_minimum_required(VERSION 3.10)

project(
    arcnet
    VERSION 0.0.1
    DESCRIPTION "networking library spin-off"
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EVENT__DISABLE_OPENSSL ON)
add_subdirectory(lib/event EXCLUDE_FROM_ALL)

add_subdirectory(lib/boost)

add_subdirectory(lib/fmt)

add_subdirectory(discovery)
