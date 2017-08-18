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

static options_t *options = NULL;

static bool options_set_defaults(options_t *ret, gru_status_t *status) {
	ret->uri = gru_uri_parse("amqp://localhost:5672/test.performance.queue", status);
	if (gru_status_error(status)) {
		fprintf(stderr, "%s", status->message);
		return false;
	}

	ret->maestro_uri = gru_uri_parse("mqtt://localhost:1883/mpt/maestro", status);
	if (gru_status_error(status)) {
		fprintf(stderr, "%s", status->message);
		return false;
	}

	ret->logdir = NULL;
	ret->parallel_count = 1;
	ret->count = 0;
	ret->log_level = GRU_INFO;
	ret->message_size = 32;
	ret->variable_size = false;
	ret->duration = gru_duration_new();
	ret->throttle = 0;
	ret->file = NULL;

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
	gru_uri_cleanup(&opt->uri);

	gru_dealloc_string(&opt->logdir);
	gru_dealloc_string(&opt->name);
	gru_dealloc_string(&opt->file);

	gru_dealloc((void **) obj);
}

void set_options_object(options_t *obj) {
	if (options == NULL) {
		options = obj;
	}
}

const options_t *get_options_object(void) {
	return options;
}

bool options_set_broker_uri(options_t *obj, const char *url, gru_status_t *status) {
	gru_uri_cleanup(&obj->uri);

	options->uri = gru_uri_parse(url, status);
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

bool options_set_name(options_t *obj, const char *name) {
	gru_dealloc_string(&obj->name);
	obj->name = strdup(name);
	if (!obj->name) {
		return false;
	}

	return true;
}

bool options_set_logdir(options_t *obj, const char *logdir) {
	gru_dealloc_string(&obj->logdir);
	obj->logdir = strdup(logdir);
	if (!obj->logdir) {
		return false;
	}

	return true;
}


bool options_set_file(options_t *obj, const char *file) {
	gru_dealloc_string(&obj->file);
	obj->file = strdup(file);
	if (!obj->file) {
		return false;
	}

	return true;
}