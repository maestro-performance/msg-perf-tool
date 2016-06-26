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
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <sys/time.h>
#include <sys/time.h>
    
typedef struct timeval mpt_timestamp_t;

mpt_timestamp_t statistics_now();
mpt_timestamp_t ts_from_milli_char(const char *ts);
mpt_timestamp_t ts_from_milli(int64_t timestamp);



#ifdef __cplusplus
}
#endif

#endif /* TIMESTAMP_H */

