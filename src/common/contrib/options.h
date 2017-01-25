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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <collection/gru_list.h>
#include <log/gru_logger.h>
#include <network/gru_uri.h>
#include <time/gru_duration.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct options_t_ {
	gru_uri_t uri;
	uint64_t count;
	char logdir[OPT_MAX_STR_SIZE];
	uint16_t parallel_count;
	size_t message_size;
	log_level_t log_level;
	gru_duration_t duration;
	bool daemon;
	uint32_t throttle;
	gru_list_t *probes;
} options_t;

options_t *options_new();
void options_destroy(options_t **obj);

void options_set_defaults(options_t *ret);

void set_options_object(options_t *ojb);
const options_t *get_options_object(void);

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */
