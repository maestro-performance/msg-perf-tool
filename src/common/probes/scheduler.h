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
 * File:   scheduler.h
 * Author: otavio
 *
 * Created on January 14, 2017, 11:15 AM
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>

#include <common/gru_status.h>
#include <collection/gru_list.h>

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct probe_entry_t_ {
    bool cancel;
} probe_entry_t;

    
void probe_scheduler_start();
void probe_scheduler_stop();


#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_H */

