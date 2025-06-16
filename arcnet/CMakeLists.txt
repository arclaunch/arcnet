cmake_minimum_required(VERSION 3.10)

project(
    arcnet
    VERSION 0.0.1
    DESCRIPTION "networking library spin-off"
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# If using windows, make it a static binary to avoid errors around nor having libgcc[..].dll, libstdcc++[..].dll
if (WIN32)
    set(INTERPROCEDURAL_OPTIMIZATION OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-allow-multiple-definition -static-libgcc -static-libstdc++") # for executable projects
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-allow-multiple-definition -static-libgcc -static-libstdc++") # for library projects
endif()

set(EVENT__DISABLE_OPENSSL ON)
add_subdirectory(lib/event EXCLUDE_FROM_ALL)

add_subdirectory(lib/boost)

add_subdirectory(lib/fmt)

add_subdirectory(discovery)
