/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "ioutils.h"

/**
 * Checks whether a given file exists
 * @param filename the filename
 * @return
 */
bool exists(const char *filename)
{
    int ret = 0;
    struct stat info;

    ret = stat(filename, &info);
    if (ret == 0) {
        return true;
    }

    return false;
}

/**
 * Checks whether can read/write a given file
 * @param filename the filename
 * @return
 */
bool can_read_write(const char *filename)
{
    int ret = 0;


    ret = access(filename, R_OK | W_OK);
    if (ret < 0) {
        logger_t logger = get_logger();
        switch (errno) {
        case ENOENT:
        {
            logger(ERROR, "No such file or directory %s: %s",
                   filename, strerror(errno));
            return false;
        }
        case EACCES:
        {
            logger(ERROR, "Access denied (no read/write permission) %s: %s",
                   filename, strerror(errno));
            return false;
        }
        default:
        {
            logger(ERROR, "Unhandled IO error trying to access %s: %s",
                   filename, strerror(errno));
            return false;
        }
        }
    }

    return true;
}

bool rename_if_exists(const char *filename)
{
    logger_t logger = get_logger();

    if (!exists(filename)) {
        return true;
    }

    if (!can_read_write(filename)) {
        return false;
    }

    int size = strlen(filename) + 16;
    char *new_file = (char *) malloc(size);

    if (!new_file) {
        logger(ERROR,
               "Not enough memory to allocate for renaming the existing file");
        return false;
    }

    int i = 0;
    do {
        bzero(new_file, size);
        snprintf(new_file, size, "%s.%03i", filename, i);

        if (!exists(new_file)) {
            int ret = 0;

            ret = rename(filename, new_file);
            if (ret != 0) {
                logger(ERROR, "Unable to rename log file: %s",
                       strerror(errno));

                free(new_file);
                return false;
            }

            break;
        }
        i++;
    }
    while (true);

    free(new_file);
    return true;
}

static char *mk_fullpath(const char *dir, const char *name) {
    char *fullpath;
    size_t size = strlen(dir) + APPEND_SIZE_REMAP;
    logger_t logger = get_logger();

    fullpath = (char *) malloc(size);
    if (fullpath == NULL) {
        logger(ERROR, "Unable to remap IO: not enough free memory");

        return false;
    }

    bzero(fullpath, size);
    snprintf(fullpath, size - 1, "%s/%s", dir, name);
    
    return fullpath;
}

bool remap_io(const char *dir, const char *name, FILE *fd)
{  
    char *fullpath = mk_fullpath(dir, name);

    if (!rename_if_exists(fullpath)) {
        free(fullpath);
        return false;
    }

    FILE *f = freopen(fullpath, "a", fd);
    if (f == NULL) {
        logger_t logger = get_logger();
        
        logger(ERROR, "Unable to remap error IO: %s (%s)", strerror(errno),
               fullpath);

        free(fullpath);
        return false;
    }

    free(fullpath);
    return true;
}

FILE *open_file(const char *dir, const char *name) {
    
    char *fullpath = mk_fullpath(dir, name);
    
     if (!rename_if_exists(fullpath)) {
        free(fullpath);
        return NULL;
    }

    
    FILE *f = fopen(fullpath, "w+");
    if (f == NULL) {
        logger_t logger = get_logger();
        logger(ERROR, "Unable to open file for IO: %s (%s)", strerror(errno),
               fullpath);

        free(fullpath);
        return false;
    }
    
    free(fullpath);
    return f;
}