#include "file.h"
#include <stdio.h>
#include <sys/stat.h>

#include "platform.h"
#include "messenger.h"

#define min_value(a, b) (a < b ? a : b)
#define max_value(a, b) (a > b ? a : b)

// just a wrapper, to help keep the file utilities organized
u64 spread_file_size(const char *path)
{
    return cutil_platform_test_file_size(path);
}

bool cutil_read_file_exists(const char *path)
{
    return cutil_platform_test_for_file(path);
}

Result cutil_read_file_binary(
    void *restrict dest, const char *restrict path, const u64 size)
{

    // localize filepath
    u32 pathLength = 0;
    cutil_platform_localize_file_name(NULL, path, &pathLength);
    char filepath[pathLength];
    cutil_platform_localize_file_name(filepath, path, &pathLength);

    db_assert_msg(size, "Size cannot have a NULL value"); // size cannot be NULL

    // open file
    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        log_error("Failed to open file '%s'.", filepath);
        return RS_FAILURE;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    const u64 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read/store contents
    if (dest)
        fread(dest, min_value(fileSize, size), 1, file);

    fclose(file);

    return RS_SUCCESS;
}

Result cutil_read_file_text(
    char *restrict dest, const char *restrict path, const u64 length)
{

    // localize filepath
    u32 pathLength = 0;
    cutil_platform_localize_file_name(NULL, path, &pathLength);
    char filepath[pathLength];
    cutil_platform_localize_file_name(filepath, path, &pathLength);

    db_assert_msg(length, "Length must have be a valid pointer");

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        log_error("Failed to open file '%s'.", filepath);
        return RS_FAILURE;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    const u64 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read file
    if (dest)
    {
        fread(dest, min_value(fileSize, length - 1), 1, file);
        dest[min_value(fileSize, length)] = '\0';
    }

    fclose(file);
    return RS_SUCCESS;
}

time_t cutil_read_file_modified_time(const char *path)
{
    return cutil_platform_get_file_modified_date(path);
}

Result
cutil_write_file_binary(const char *path, const void *contents, const u32 size)
{
    // localize filepath
    u32 pathLength = 0;
    cutil_platform_localize_file_name(NULL, path, &pathLength);
    char filepath[pathLength];
    cutil_platform_localize_file_name(filepath, path, &pathLength);

    FILE *file = fopen(filepath, "w");
    if (!file)
        return RS_FAILURE;

    db_assert_msg(ftell(file) == 0, "File not at start");

    if (fwrite(contents, size, 1, file) == 0 && ferror(file))
    {
        log_error("Failed to write data to file '%s'", path);
        return RS_FAILURE;
    }

    if (fclose(file))
        return RS_FAILURE;
    return RS_SUCCESS;
}

Result cutil_write_file_folder(const char *path)
{
    return cutil_platform_create_folder(path);
}
