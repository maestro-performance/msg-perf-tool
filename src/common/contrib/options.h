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

typedef struct options_t_ options_t;

options_t *options_new(gru_status_t *status);
void options_destroy(options_t **obj);

void set_options_object(options_t *ojb);

bool options_set_broker_uri(options_t *obj, const char *url, gru_status_t *status);
bool options_set_maestro_uri(options_t *obj, const char *url, gru_status_t *status);
bool options_set_name(options_t *obj, const char *name);
bool options_set_logdir(options_t *obj, const char *logdir);
void options_set_log_level(options_t *obj, const char *logdir);
bool options_set_file(options_t *obj, const char *file);
bool options_set_duration(options_t *obj, const char *value);
void options_set_parallel_count(options_t *obj, const char *value);
void options_set_message_count(options_t *obj, const char *value);
void options_set_message_size(options_t *obj, const char *value);
void options_set_throttle(options_t *obj, const char *value);

const gru_uri_t options_get_broker_uri();

const char *options_get_maestro_host();
const gru_uri_t options_get_maestro_uri();

const char *options_get_name();
const char *options_get_log_dir();
const char *options_get_maestro_script();
uint16_t options_get_parallel_count();
uint64_t options_get_count();
uint32_t options_get_throttle();

gru_duration_t options_get_duration();
log_level_t options_get_log_level();
size_t options_get_message_size();
bool options_get_variable_size();

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */
