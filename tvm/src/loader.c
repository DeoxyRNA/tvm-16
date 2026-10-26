#if defined(_WIN32)
    #include <windows.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <errno.h>
#else
    #error "Unsupported platform - compile with GCC or Clang on Windows, Linux, or macOS"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tvm/cpu_spec.h>
#include <tvm/cpu_procs.h>
#include <tvm/env.h>
#include "vm.h"

static const char* INVALID_FORMAT_MSG = "%s: fatal error: input file must be a valid TVM-16 executable\n";
static const char* FILE_TOO_LARGE_MSG = "%s: fatal error: input file too large! (max 64 KiB)\n";
static const char* FILE_NOT_FOUND_MSG = "%s: cannot find %s: No such file or directory\n";
static const char* PERMISSION_DENIED_MSG = "%s: cannot open %s: Permission denied\n";

static void* createReadonlyFileView(const char* restrict filename, uint64_t* restrict size, uint64_t* restrict error) {
#if defined(_WIN32)
    HANDLE file = CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        if (error) *error = (uint64_t)GetLastError();
        if (size) *size = 0;
        return NULL;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(file, &fileSize)) {
        if (error) *error = (uint64_t)GetLastError();
        if (size) *size = 0;
        CloseHandle(file);
        return NULL;
    }

    if (fileSize.QuadPart == 0) {
        if (error) *error = 0;
        if (size) *size = 0;
        CloseHandle(file);
        return NULL;
    }

    HANDLE mapping = CreateFileMappingA(file, NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle(file);

    if (mapping == NULL) {
        if (error) *error = (uint64_t)GetLastError();
        if (size) *size = 0;
        return NULL;
    }

    void* data = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(mapping);

    if (data == NULL) {
        if (error) *error = (uint64_t)GetLastError();
        if (size) *size = 0;
        return NULL;
    }

    if (size) *size = (uint64_t)fileSize.QuadPart;
    if (error) *error = 0;
    return data;
#else
    int fileDesc = open(filename, O_RDONLY);
    if (fileDesc == -1) {
        if (error) *error = (uint64_t)errno;
        if (size) *size = 0;
        return NULL;
    }

    struct stat fileStat;
    if (fstat(fileDesc, &fileStat) == -1) {
        if (error) *error = (uint64_t)errno;
        if (size) *size = 0;
        close(fileDesc);
        return NULL;
    }

    if (fileStat.st_size <= 0) {
        close(fileDesc);
        if (error) *error = 0;
        if (size) *size = 0;
        return NULL;
    }

    if ((uint64_t)fileStat.st_size > (uint64_t)SIZE_MAX) {
        close(fileDesc);
        if (error) *error = (uint64_t)EFBIG;
        if (size) *size = 0;
        return NULL;
    }

    void* data = mmap(NULL, (size_t)fileStat.st_size, PROT_READ, MAP_SHARED, fileDesc, 0);
    close(fileDesc);

    if (data == MAP_FAILED) {
        if (error) *error = (uint64_t)errno;
        if (size) *size = 0;
        return NULL;
    }

    if (size) *size = (uint64_t)fileStat.st_size;
    if (error) *error = 0;
    return data;
#endif
}

static void destroyReadonlyFileView(const void* restrict view, uint64_t size) {
    if (view == NULL || size == 0) return;
#if defined(_WIN32)
    UnmapViewOfFile(view);
#else
    if (size <= (uint64_t)SIZE_MAX) {
        munmap((void*)view, (size_t)size);
    }
#endif
}

int tvmLoadExecutable(const char* path) {
    uint64_t fileSize = 0;
    uint64_t error = 0;
    void* fileView = createReadonlyFileView(path, &fileSize, &error);

    if (fileView == NULL) {
#if defined(_WIN32)
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            fprintf(stderr, FILE_NOT_FOUND_MSG, INVOCATION_NAME, path);
        }
#else
        if (error == ENOENT || error == ENOTDIR) {
            fprintf(stderr, FILE_NOT_FOUND_MSG, INVOCATION_NAME, path);
        }
#endif
        else if (fileSize == 0) {
            fprintf(stderr, INVALID_FORMAT_MSG, INVOCATION_NAME, path);
        } else {
            fprintf(stderr, PERMISSION_DENIED_MSG, INVOCATION_NAME, path);
        }

        return EXIT_FAILURE;
    }

    if (fileSize > 65536u) {
        fprintf(stderr, FILE_TOO_LARGE_MSG, INVOCATION_NAME, path);
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    if (fileSize < 8) {
        fprintf(stderr, INVALID_FORMAT_MSG, INVOCATION_NAME, path);
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    const uint8_t* fileBytes = fileView;
    if (memcmp(fileBytes, "pTVM", 4) != 0) {
        fprintf(stderr, INVALID_FORMAT_MSG, INVOCATION_NAME, path);
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    uint16_t inputOffset = 4; // skip the magic number
    uint16_t outputOffset = 0;

    uint16_t staticSegmentSize = TVM_READ_U16_LENDIAN(fileBytes + inputOffset);
    inputOffset += 2;
    uint16_t dataSegmentSize = TVM_READ_U16_LENDIAN(fileBytes + inputOffset);
    inputOffset += 2;

    // allocate bytes for static segment
    outputOffset += staticSegmentSize;

    if (outputOffset + dataSegmentSize > fileSize) {
        fprintf(stderr, INVALID_FORMAT_MSG, INVOCATION_NAME, path);
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    // copy data segment from input file to memory
    memcpy(memory + outputOffset, fileBytes + inputOffset, dataSegmentSize);
    outputOffset += dataSegmentSize;

    outputOffset = TVM_CODE_SEGMENT_OFFSET;
    inputOffset += dataSegmentSize;

    uint16_t codeSegmentSize = fileSize - inputOffset;

    // copy code segment from input file to memory
    memcpy(memory + outputOffset, fileBytes + inputOffset, codeSegmentSize);

    destroyReadonlyFileView(fileView, fileSize);

    return EXIT_SUCCESS;
}