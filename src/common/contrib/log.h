/**
 Copyright 2015 Otavio Rodolfo Piske
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#ifndef LOG_H
#define	LOG_H

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
#include "options.h"
#include "ioutils.h"

#ifdef	__cplusplus
extern "C" {
#endif

bool remap_log(const char *dir, const char *base_name, pid_t parent, pid_t pid, FILE *fd);
void default_logger(log_level_t level, const char *msg, ...);

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */

