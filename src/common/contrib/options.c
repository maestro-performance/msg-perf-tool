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
#include "options.h"

struct options_t_ {
    gru_uri_t broker_uri;
    gru_uri_t maestro_uri;
    char *log_dir;
    log_level_t log_level;

    uint16_t parallel_count;
    size_t message_size;
    bool variable_size;

    uint64_t count;
    gru_duration_t duration;
    uint32_t throttle;
    char *name;
    char *maestro_script;
};

static struct options_t_ *options = NULL;

static bool options_set_defaults(options_t *ret, gru_status_t *status) {
	ret->broker_uri = gru_uri_parse("amqp://localhost:5672/test.performance.queue", status);
	if (gru_status_error(status)) {
		fprintf(stderr, "%s", status->message);
		return false;
	}

	ret->maestro_uri = gru_uri_parse("mqtt://localhost:1883/mpt/maestro", status);
	if (gru_status_error(status)) {
		fprintf(stderr, "%s", status->message);
		return false;
	}

	ret->log_dir = NULL;
	ret->parallel_count = 1;
	ret->count = 0;
	ret->log_level = GRU_INFO;
	ret->message_size = 32;
	ret->variable_size = false;
	ret->duration = gru_duration_new();
	ret->throttle = 0;
	ret->maestro_script = NULL;

	char hostname[256] = {0};
	if (gethostname(hostname, sizeof(hostname)) == 0) {
		ret->name = strdup(hostname);
	}
	else {
		ret->name = strdup("undefined");
	}

	return true;
}

options_t *options_new(gru_status_t *status) {
	options_t *ret = gru_alloc(sizeof(options_t), status);
	gru_alloc_check(ret, NULL);

	if (!options_set_defaults(ret, status)) {
		gru_dealloc((void **) &ret);

		return NULL;
	}

	return ret;
}

void options_destroy(options_t **obj) {
	options_t *opt = (*obj);

	if (!opt) {
		return;
	}

	gru_uri_cleanup(&opt->maestro_uri);
	gru_uri_cleanup(&opt->broker_uri);

	gru_dealloc_string(&opt->log_dir);
	gru_dealloc_string(&opt->name);
	gru_dealloc_string(&opt->maestro_script);

	gru_dealloc((void **) obj);
}

void set_options_object(options_t *obj) {
	if (options == NULL) {
		options = obj;
	}
}

bool options_set_broker_uri(options_t *obj, const char *url, gru_status_t *status) {
	gru_uri_cleanup(&obj->broker_uri);

	options->broker_uri = gru_uri_parse(url, status);
	if (gru_status_error(status)) {
		return false;
	}

	return true;
}

bool options_set_maestro_uri(options_t *obj, const char *url, gru_status_t *status) {
	gru_uri_cleanup(&obj->maestro_uri);

	options->maestro_uri = gru_uri_parse(url, status);
	if (gru_status_error(status)) {
		return false;
	}

	return true;
}

void options_set_throttle(options_t *obj, const char *value) {
    options->throttle = atoi(value);
}


uint32_t options_get_throttle() {
    return options->throttle;
}


const char *options_get_maestro_host() {
    return options->maestro_uri.host;
}

const gru_uri_t options_get_maestro_uri() {
    return options->maestro_uri;
}

const gru_uri_t options_get_broker_uri() {
    return options->broker_uri;
}

bool options_set_name(options_t *obj, const char *name) {
	gru_dealloc_string(&obj->name);
	obj->name = strdup(name);
	if (!obj->name) {
		return false;
	}

	return true;
}

const char *options_get_name() {
	return options->name;
}

const char *options_get_maestro_script() {
    return options->maestro_script;
}

bool options_set_logdir(options_t *obj, const char *logdir) {
	gru_dealloc_string(&obj->log_dir);
	obj->log_dir = strdup(logdir);
	if (!obj->log_dir) {
		return false;
	}

	return true;
}


const char *options_get_log_dir() {
	return options->log_dir;
}

void options_set_log_level(options_t *obj, const char *value) {
    options->log_level = gru_logger_get_level(value);

	gru_logger_set_minimum(options->log_level);
}

bool options_set_file(options_t *obj, const char *file) {
	gru_dealloc_string(&obj->maestro_script);
	obj->maestro_script = strdup(file);
	if (!obj->maestro_script) {
		return false;
	}

	return true;
}

bool options_set_duration(options_t *obj, const char *value) {
	return gru_duration_parse(&options->duration, value);
}

void options_set_parallel_count(options_t *obj, const char *value) {
	options->parallel_count = (uint16_t) atoi(value);
}

void options_set_message_count(options_t *obj, const char *value) {
	options->count = strtol(value, NULL, 10);
}


void options_set_message_size(options_t *obj, const char *value) {
    if (value[0] == '~') {
        options->message_size = atoi(value + 1);

        options->variable_size = true;
    } else {
        options->message_size = atoi(value);
    }
}

size_t options_get_message_size() {
    return options->message_size;
}

bool options_get_variable_size() {
	return options->variable_size;
}

uint16_t options_get_parallel_count() {
    return options->parallel_count;
}

uint64_t options_get_count() {
	return options->count;
}

gru_duration_t options_get_duration() {
    return options->duration;
}

log_level_t options_get_log_level() {
    return options->log_level;
}