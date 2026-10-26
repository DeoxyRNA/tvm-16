add_executable(tasm)
target_sources(tasm
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_include_directories(tasm
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include
)