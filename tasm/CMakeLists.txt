project(TVM-16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tasm)
target_sources(tasm
PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

target_include_directories(tasm
PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)