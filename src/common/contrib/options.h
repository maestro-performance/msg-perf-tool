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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include <collection/gru_list.h>
#include <config/gru_config.h>
#include <log/gru_logger.h>
#include <network/gru_uri.h>
#include <time/gru_duration.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct options_t_ {
	gru_uri_t broker_uri;
	gru_uri_t maestro_uri;
	uint64_t count;
	char *logdir;
	uint16_t parallel_count;
	size_t message_size;
	bool variable_size;
	log_level_t log_level;
	gru_duration_t duration;
	uint32_t throttle;
	char *name;
  	char *file;
} options_t;

options_t *options_new(gru_status_t *status);
void options_destroy(options_t **obj);

void set_options_object(options_t *ojb);
const options_t *get_options_object(void);

bool options_set_broker_uri(options_t *obj, const char *url, gru_status_t *status);
bool options_set_maestro_uri(options_t *obj, const char *url, gru_status_t *status);
bool options_set_name(options_t *obj, const char *name);
bool options_set_logdir(options_t *obj, const char *logdir);
bool options_set_iface(options_t *obj, const char *iface);
bool options_set_probes(options_t *obj, const char *probes, gru_status_t *status);
bool options_set_file(options_t *obj, const char *file);

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */
