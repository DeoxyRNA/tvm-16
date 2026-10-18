project(TVM-16)

add_executable(tvm)
target_sources(tvm
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)

target_include_directories(tvm
PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)