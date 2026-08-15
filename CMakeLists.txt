cmake_minimum_required(VERSION 3.10)
project(TVM-16)

add_executable(tvm16)

target_sources(tvm16
PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/shell.c"
)

target_include_directories(tvm16
PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)