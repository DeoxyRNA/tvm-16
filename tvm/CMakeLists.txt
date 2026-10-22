project(TVM-16)

add_executable(tvm)
target_sources(tvm
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sys.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/instructions/instr_control.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/instructions/instr_branch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/instructions/instr_alu.c
)

target_include_directories(tvm
PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)