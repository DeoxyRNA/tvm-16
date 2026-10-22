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
#include "vm.h"

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
            fprintf(stderr, "error: at '%s', no such file exists\n", path);
        }
#else
        if (error == ENOENT || error == ENOTDIR) {
            fprintf(stderr, "error: at '%s', no such file exists\n", path);
        }
#endif
        else if (fileSize == 0) {
            fprintf(stderr, "error: file at '%s' contains no data\n", path);
        } else {
            fprintf(stderr, "error: could not access file '%s'\n", path);
        }

        return EXIT_FAILURE;
    }

    if (fileSize > 65536u) {
        fprintf(stderr, "error: file too large! (max 64 KiB)\n");
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    if (fileSize < 8) {
        fprintf(stderr, "error: input must be a valid TVM-16 executable\n");
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    const uint8_t* fileBytes = fileView;
    if (memcmp(fileBytes, "pTVM", 4) != 0) {
        fprintf(stderr, "error: input must be a valid TVM-16 executable\n");
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    uint16_t staticSize;
    uint16_t dataSize;
    memcpy(&staticSize, fileBytes + 4, sizeof(staticSize));
    memcpy(&dataSize, fileBytes + 6, sizeof(dataSize));

    uint32_t codeOffset = (uint32_t)staticSize + dataSize;
    if (codeOffset > fileSize) {
        fprintf(stderr, "error: input must be a valid TVM-16 executable\n");
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    uint32_t codeSize = (uint32_t)fileSize - codeOffset;
    if (codeSize > TVM_CODE_SEGMENT_SIZE) {
        fprintf(stderr, "error: program code too large! (max 32 KiB)\n");
        destroyReadonlyFileView(fileView, fileSize);
        return EXIT_FAILURE;
    }

    memcpy(memory + TVM_CODE_SEGMENT_OFFSET, fileBytes + codeOffset, codeSize);
    destroyReadonlyFileView(fileView, fileSize);
    return EXIT_SUCCESS;
}