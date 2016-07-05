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
#include "log.h"

bool remap_log(const char *dir, const char *base_name, pid_t parent, pid_t pid, FILE *fd)
{
    char name[64];

    bzero(name, sizeof (name));

    if (parent == 0) {
        snprintf(name, sizeof (name) - 1, "%s-%d.log", base_name, pid);
    }
    else {
        snprintf(name, sizeof (name) - 1, "%s-%d-%d.log", base_name, parent, pid);
    }

    return remap_io(dir, name, fd);
}

void default_logger(log_level_t level, const char *msg, ...)
{
    const options_t *options = get_options_object();

    if (!can_log(level, options->log_level)) {
        return;
    }

    va_list ap;
    char *ret = NULL;

    va_start(ap, msg);
    vasprintf(&ret, msg, ap);
    va_end(ap);


    switch (level) {
    case TRACE:
        fprintf(stderr, "[TRACE]: %s\n", ret);
        break;
    case DEBUG:
        fprintf(stderr, "[DEBUG]: %s\n", ret);
        break;
    case INFO:
        fprintf(stderr, "[INFO]: %s\n", ret);
        break;
    case STAT:
        fprintf(stderr, "[STAT]: %s\n", ret);
        break;
    case WARNING:
        fprintf(stderr, "[WARNING]: %s\n", ret);
        break;
    case ERROR:
        fprintf(stderr, "[ERROR]: %s\n", ret);
        break;
    case FATAL:
        fprintf(stderr, "[FATAL]: %s\n", ret);
        break;
    default:
        fprintf(stderr, "[MSG]: %s\n", ret);
        break;
    }

    free(ret);
}