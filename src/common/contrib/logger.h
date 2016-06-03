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

#ifndef CONTRIB_LOGGER_H_
#define CONTRIB_LOGGER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

typedef enum log_level_t_ {
	TRACE,
	DEBUG,
	INFO,
      	STAT,
	WARNING,
	ERROR,
	FATAL,
} log_level_t;

typedef void(*logger_t)(log_level_t level, const char *message, ...);

log_level_t get_log_level(const char *str);
bool can_log(log_level_t l1, log_level_t l2);

void set_logger(logger_t new_msg);
logger_t get_logger(void);

//CONTRIB_LOGGER_H_
#endif
