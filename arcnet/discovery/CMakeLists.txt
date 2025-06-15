set(PROJECT arcnet_discovery)

project(
    ${PROJECT}
    VERSION ${CMAKE_PROJECT_VERSION}
    LANGUAGES CXX
)

add_library(${PROJECT})

add_library(arcnet::discovery ALIAS ${PROJECT})

target_sources(${PROJECT}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/mdns/mdns_emitter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/mdns/service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/interfaces.cpp
)

target_include_directories(${PROJECT}
     PRIVATE
          # internal headers
          ${CMAKE_CURRENT_SOURCE_DIR}/src
     PUBLIC
          # exported headers
          ${CMAKE_CURRENT_SOURCE_DIR}/include
)


set(MDNS_BUILD_EXAMPLE OFF)
add_subdirectory(lib/mdns EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT} PUBLIC mdns event_core_shared event_shared Boost::asio fmt::fmt)

add_executable(${PROJECT}_test)
target_sources(${PROJECT}_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(${PROJECT}_test PUBLIC ${PROJECT})