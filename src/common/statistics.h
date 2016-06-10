/**
 Copyright 2016 Otavio Rodolfo Piske
 
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
#ifndef STATISTICS_H
#define STATISTICS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "contrib/logger.h"
    
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
    
typedef struct timeval mpt_timestamp_t;

void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end);
uint64_t statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end);
mpt_timestamp_t statistics_now();


#ifdef __cplusplus
}
#endif

#endif /* STATISTICS_H */

