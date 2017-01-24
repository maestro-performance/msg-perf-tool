/*
 * Copyright 2017 otavio.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * File:   probe.h
 * Author: otavio
 *
 * Created on January 14, 2017, 11:15 AM
 */

#ifndef PROBE_H
#define PROBE_H

#include <common/gru_status.h>

#include "contrib/options.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*probe_init)(const options_t *options, gru_status_t *status);
typedef int (*probe_collect)(gru_status_t *status);
typedef void (*probe_stop)();
typedef const char *(*probe_name)();

typedef struct probe_entry_t_ {
    probe_init init;
    probe_collect collect;
    probe_stop stop;
    probe_name name;

    pthread_t thread;
    void *handle;
    bool cancel;
} probe_entry_t;


#ifdef __cplusplus
}
#endif

#endif /* PROBE_H */

