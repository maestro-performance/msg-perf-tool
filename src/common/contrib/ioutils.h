/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ioutils.h
 * Author: otavio
 *
 * Created on July 5, 2016, 11:23 AM
 */

#ifndef IOUTILS_H
#define IOUTILS_H

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "logger.h"

#define APPEND_SIZE_REMAP 64

#ifdef __cplusplus
extern "C" {
#endif

bool can_read_write(const char *filename);
bool rename_if_exists(const char *filename);
bool remap_io(const char *dir, const char *name, FILE *fd);

FILE *open_file(const char *dir, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* IOUTILS_H */

