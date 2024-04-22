#include "../platform.h"

#ifdef __unix__

#include <sys/cdefs.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <ftw.h>
#include <string.h>
#include "../types.h"
#include "../messenger.h"

#include "../string_util.h"

/**
 * @brief Localize a file name. This macro is more convientent then writing the
 * same code over and over again.
 *
 * @param inputpath the variable storing the path to localize
 * @param outputpathname the name of the localized path name. the macro
 * creates it.
 * @param outputpathlength the name of the variable storing the localized
 * filepaths length. the macro creates it
 *
 */
#define localize_path(inputPath, outputPathName, outputPathLength)         \
    u32 outputPathLength = 0;                                              \
    cutil_platform_localize_file_name(NULL, inputPath, &outputPathLength); \
    char outputPathName[outputPathLength];                                 \
    cutil_platform_localize_file_name(                                     \
        outputPathName, inputPath, &outputPathLength);

#define assert_executable_directory_set()                                      \
    if (g_executableDirectory == NULL)                                         \
    {                                                                          \
        log_fatal(                                                             \
            "The executable directory has not been set. Some cutils\n"         \
            "require that the executable directory is set using\n"             \
            "util_platform_set_executable_directory(argv[0]) in order to "     \
            "verify\n"                                                         \
            "that file operations are safe, for the developer and the user."); \
        abort();                                                               \
    }

#define assert_allowed_file_operation(path)              \
    if (!cutil_platform_is_allowed_file_operation(path)) \
        abort();

// store the exectutable files directory
// so that assets and other relative directories
// can be located during runtime
char g_executableDirectory[1024];
u32 g_executableDirectoryLength = 0;

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

void cutil_platform_set_executable_folder(char *name)
{

    // remove executable fild name from the path
    u32 pathLength = 0;
    cutil_string_truncate(NULL, &pathLength, name, '/', true);
    db_assert_msg(
        pathLength < sizeof(g_executableDirectory), "Executable path too long");
    g_executableDirectoryLength = --pathLength;
    pathLength                  = sizeof(g_executableDirectory);
    cutil_string_truncate(g_executableDirectory, &pathLength, name, '/', true);

    db_assert_msg(
        g_executableDirectoryLength == strlen(g_executableDirectory), "");
}

const char *cutil_platform_get_executable_folder(void)
{
    return g_executableDirectory;
}

u32 cutil_platform_get_executable_folde_str_len(void)
{
    return g_executableDirectoryLength;
}

Result
cutil_platform_localize_file_name(char *output, const char *path, u32 *max)
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
            if (output[i] == '\\')
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
        if (output[i] == '\\')
            output[i] = CUTIL_PLATFORM_FOLDER_BREAK;
    }

    output && (output[*max - 1] = '\0');

    return RS_SUCCESS;
}

char *cutil_platform_get_cwd(void) { return getcwd(NULL, 0); }

bool cutil_platform_test_for_file(const char *filepath)
{

    localize_path(filepath, path, pathLength);

    if (access(path, F_OK | R_OK) == -1)
    {
        log_perror("Failed to test for file '%s'", path);
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

f64 cutil_platform_get_time(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1.0e6;
}

time_t cutil_platform_get_file_modified_date(const char *filepath)
{
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
    localize_path(filepath, path, pathLength);

    assert_allowed_file_operation(path);

    if (path[pathLength - 2] == CUTIL_PLATFORM_FOLDER_BREAK)
        path[pathLength - 2] = '\0';

    for (char *p = path + 1; *p != '\0'; p++)
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

    int n = 0;
    struct dirent *d;
    DIR *dir = opendir(path);
    if (dir == NULL) // not a directory or doesn't exist
        return 1;
    while ((d = readdir(dir)) != NULL)
    {
        if (++n > 2)
            break;
    }
    closedir(dir);
    if (n <= 2) // Directory Empty
        return true;
    else
        return false;
}

int ftw_deleteFolderCallback(
    const char *fpath,
    const struct stat *sb __attribute_maybe_unused__,
    int typeflag)
{
    if (typeflag & FTW_D)
    {
        DIR *folder = opendir(fpath);
        if (readdir(folder))
        {
            return 0; // shouldn't delete folders
        }
    }

    remove(fpath);
    return 0;
}

Result cutil_platform_delete_folder(const char *restrict filepath)
{
    localize_path(filepath, path, pathLength);

    assert_allowed_file_operation(path);

    // get rid of the slash so the rest of the function works properly
    if (path[pathLength - 2] == CUTIL_PLATFORM_FOLDER_BREAK)
        path[pathLength - 2] = '\0';

    ftw(path, &ftw_deleteFolderCallback, 256);

    if (cutil_platform_is_directory_empty(path))
    {
        remove(path);
        return RS_SUCCESS;
    }

    return RS_FAILURE;
}

Result cutil_platform_delete_file(const char *restrict filepath)
{
    localize_path(filepath, path, pathLength);

    assert_allowed_file_operation(path);

    // check to be sure folders are empty
    struct stat file_stats;
    if (stat(path, &file_stats) == -1)
    {
        if (errno != EEXIST)
            log_perror("%s", path);
        return RS_FAILURE;
    }
    if (!S_ISREG(file_stats.st_mode))
    {
        if (!cutil_platform_is_directory_empty(path))
        {
            log_warning("Cannot delete file that is not empty");
            return RS_FAILURE;
        }
    }

    if (remove(path) == -1)
    {
        log_perror("%s", path);
        return RS_FAILURE;
    }
    return RS_SUCCESS;
}

#endif
