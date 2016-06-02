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
#ifndef OPTIONS_H
#define OPTIONS_H

#include "defaults.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct options_t_ {
    char url[OPT_MAX_STR_SIZE];
    unsigned long long int count;
    char command[OPT_MAX_STR_SIZE];
    char logdir[OPT_MAX_STR_SIZE];
    bool daemon;
    bool debug;
    bool trace;
} options_t;


options_t *options_new();
void set_options_object(options_t *ojb);
const options_t *get_options_object(void);

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */

