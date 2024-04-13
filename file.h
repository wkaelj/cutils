#pragma once
#include "types.h"
// Utilities usefull in file I/O
//
//
// Kael Johnston May 1 2022

// ===================================
//              Reading
// ===================================

/**
 * @brief Read the size of a file
 *
 * @param filepath the path to the file
 * @return u64 will be 0 for failure or if the file does not exist
 */
u64 cutil_read_file_size(const char *filepath);

/**
 * @brief Test if a file exists. This function has no performance benefit
 * compared to spReadFileSize
 *
 * @param filepath
 * @return true the file exists
 * @return false the file does not exist
 */
bool cutil_read_file_exists(const char *filepath);

/**
 * @brief Read a file into the memory of dest, with a maximum size of size.
 * If the file is larger then size, it will be truncated at size. You can check
 * the size of the file using the spReadFileSize function.
 *
 * @param dest the file will be read into this pointer, with no alignment.
 * @param filepath the file to read
 * @param size the maximum number of bytes to read.
 * @return Result
 */
Result cutil_read_file_binary(
    void *restrict dest, const char *restrict filepath, const u64 size);

/**
 * @brief Read the contents of a text file into the string pointed to by dest.
 *
 * @param dest the file will be written here
 * @param path the file to read
 * @param length the mamixmum length of the string (including terminator)
 * @return Result
 */
Result cutil_read_file_text(
    char *restrict dest, const char *restrict path, const u64 length);

/**
 * @brief get the last time a file was modified
 *
 * @param filepath
 * @return time_t
 */
time_t cutil_read_file_modified_time(const char *filepath);

// ===================================
//              Writing
// ===================================

/**
 * @brief Write the contents of a pointer to a file.
 *
 * @param path the file path
 * @param contents the data to write
 * @param size the amount of data to write
 * @return Result
 */
Result cutil_write_file_binary(
    const char *restrict path, const void *restrict contents, const u32 size);

/**
 * @brief Create a folder with the name path. It is recursive.
 *
 * @param path the folder to create
 * @return Result
 */
Result cutil_write_file_folder(const char *path);
