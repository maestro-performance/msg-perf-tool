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

#include "scheduler.h"

pthread_t thread;
probe_entry_t sentry;

static void *probe_scheduler_run(void *en) {
    probe_entry_t *entry = (probe_entry_t *) en;
    
    while (!entry->cancel) {
        printf("Running ...\n");
        sleep(1);
    }
    
    
    printf("Finished running ...\n");
    return NULL;
}


void probe_scheduler_start() {
    int sys_ret = pthread_create(&thread, NULL, probe_scheduler_run, &sentry);
    if (sys_ret != 0) {
        printf("Unable to create probe thread\n");
        
        return;
    }
    
    printf("Created\n");
}


void probe_scheduler_stop() {
    printf("Stopping\n");
    sentry.cancel = true;
}