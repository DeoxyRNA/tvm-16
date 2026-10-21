#ifndef TVM_SRC_VM_H
#define TVM_SRC_VM_H

/// @brief Loads the specified TVM-16 executable into memory.
/// @param path The path to the executable (a null-terminated string).
/// @return EXIT_SUCCESS on success, EXIT_SUCCESS on failure.
int tvmLoadExecutable(const char* path);

/// @brief Runs the TVM-16 executable currently loaded into memory.
/// @param argv A list of command-line arguments to pass to the program (an array of null-terminated strings).
/// @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
int tvmRun(char** argv);

#endif // TVM_SRC_VM_H