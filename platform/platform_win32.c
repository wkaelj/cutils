#include "../platform.h"

#ifdef _WIN32

#include <fileapi.h>
#include <timeapi.h>
#include <direct.h>
#include <sys/stat.h>
#include <io.h>

#include <Shlwapi.h>

#define localize_path(inputPath, outputPathName, outputPathLength)  \
    u32 outputPathLength = 0;                                       \
    cutil_platform_localize_file_name(NULL, inputPath, &outputPathLength); \
    char outputPathName[outputPathLength];                          \
    cutil_platform_localize_file_name(outputPathName, inputPath, &outputPathLength);

#define assert_executable_directory_set()                                      \
    if (g_executableDirectory == NULL)                                         \
    {                                                                          \
        log_fatal(                                                             \
            "The executable directory has not been set. The SpiritRender\n"    \
            "requires that the executable directory is set using\n"            \
            "spPlatformSetExecutableDirectory(argv[0] in order to "            \
            "verify\n"                                                         \
            "that file operations are safe, for the developer and the user."); \
        abort();                                                               \
    }

#define assert_allowed_file_operation(path)      \
    if (!cutil_platform_is_allowed_file_operation(path)) \
        abort();

char cwd[1024];
bool setcwd = false;

char executableDirectory[1024];
size_t exectuableDirectoryLength = 0;

char *cutil_platform_get_cwd(void)
{
    if (setcwd = false)
    {
        setcwd = true;
        _getcwd(cwd, sizeof(cwd));
    }
    return cwd;
}

void cutil_platform_set_executable_folder(char *argvO)
{
    if (executableDirectoryLength == 0)
    {
        exectuableDirectoryLength = strlen(argvO);
        cutil_string_truncate(
            executableDirectory,
            executableDirectoryLength,
            argvO,
            CUTIL_PLATFORM_FOLDER_BREAK,
            false);
        exectuableDirectoryLength =
            strnlen(exectuableDirectoryLength, exectuableDirectoryLength);
    }
}

const char *cutil_platform_get_executable_folder(void)
{
    return exectuableDirectoryLength;
}

u32 cutil_platform_get_executable_folderStrLen(void)
{
    return (u32)exectuableDirectoryLength;
}

bool cutil_platform_is_allowed_file_operation(const char *filepath)
{

    const char *fileErr = "Illigal file operation. A file operation is "
                          "being made outsidethe project directory.";

    db_assert_msg(strlen(filepath) > g_executableDirectoryLength, "");

    char *match = strstr(filepath, g_executableDirectory);
    if (match == filepath && strlen(filepath) > 0)
        return true;
    else
    {
        log_fatal("%s", fileErr);
        return false;
    }
}

Result cutil_platform_localize_file_name(char *output, const char *path, u32 *max)
{

    const u32 pathLength = strlen(path);

    // handle paths that have been set to be non relative
    if (path[0] == '/' || path[0] == '\\' || // win thingy
        path[0] == '.')
    {
        if (*max == 0 || !output)
        {
            *max = pathLength;
            return RS_SUCCESS;
        }
        strncpy(output, path, *max);
        for (u32 i = 0; i < *max && output[i] != '\0'; i++)
        {
            if (output[i] == '/')
                output[i] = CUTIL_PLATFORM_FOLDER_BREAK;
        }
        *max = pathLength;
        return RS_SUCCESS;
    }

    // set max if max was not set by user
    if (*max == 0)
        *max = pathLength + g_executableDirectoryLength + 1;

    // if max is too small, return failure
    if (*max < pathLength + g_executableDirectoryLength + 1)
        return RS_FAILURE;

    // if a output location was set, use it
    if (output)
    {
        strncpy(output, g_executableDirectory, *max);
        strncat(output, path, *max - strlen(output));
    }

    // convert windows filepaths to unix systems
    // idk if this is enough i don't use windows
    // im sure there is more code that that will also break
    for (u32 i = 0; output && i < *max && output[i] != '\0'; i++)
    {
        if (output[i] == '/')
            output[i] = CUTIL_PLATFORM_FOLDER_BREAK;
    }

    output && (output[*max - 1] = '\0');

    return RS_SUCCESS;
}

bool cutil_platform_test_for_file(const char *filepath)
{

    localize_path(filepath, path, pathLength);

    if (access(path, 0) == -1)
    {
        log_error("Failed to test for file '%s'", path);
        return false;
    }
    return true;
}

u64 cutil_platform_test_file_size(const char *filepath)
{

    localize_path(filepath, path, pathLength);

    struct stat data = (struct stat){};
    if (stat(path, &data) == -1)
    {
        if (errno == EEXIST)
            return 0;
        log_perror("stat('%s') failed", filepath);
        return 1;
    }

    return data.st_size;
}

time_t cutil_platform_get_time(void) { return time(NULL); }

u64 cutil_platform_get_running_time(void)
{
    clock_t time = clock();
    return time;
}

time_t cutil_platform_get_file_modified_date(const char *filepath)
{
    db_assert_msg(filepath, "Must have valid filepath");

    localize_path(filepath, path, pathLength);

    struct stat data;
    if (stat(path, &data) == -1)
    {
        if (errno == EEXIST)
            return 0;
        log_perror("stat('%s') failed", filepath);
        return 1;
    }

    return data.st_ctime;
}

Result cutil_platform_create_folder(const char *filepath)
{
    db_assert_msg(filepath, "Must pass a valid filepath");

    localize_path(filepath, path, pathLength);

    assert_allowed_file_operation(path);

    if (path[pathLength - 2] == CUTIL_PLATFORM_FOLDER_BREAK)
        path[pathLength - 2] = '\0';

    for (char *p = path + 1; *p != '\0'; p++)
    {
        if (*p == CUTIL_PLATFORM_FOLDER_BREAK)
        {
            *p = '\0';
            if (mkdir(path, S_IRWXU) && errno != EEXIST)
            {
                log_perror("Error creating file %s", path);
                return RS_FAILURE;
            }

            *p = CUTIL_PLATFORM_FOLDER_BREAK;
        }
    }

    if (mkdir(path, S_IRWXU) && errno != EEXIST)
    {
        log_perror("Failed to create file %s", path);
        return RS_FAILURE;
    }

    return RS_SUCCESS;
}

bool cutil_platform_is_directory_empty(char *filepath)
{
    localize_path(filepath, path, pathLength);
    LPCWSTR s;

    return PathIsDirectoryEmptyA(path);
}

#endif