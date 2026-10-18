project(TVM-16)

add_executable(tasm)
target_sources(tasm
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)

target_include_directories(tasm
PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)