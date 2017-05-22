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

void options_set_defaults(options_t *ret) {
	gru_status_t status = gru_status_new();

	ret->uri = gru_uri_parse("amqp://localhost:5672/test.performance.queue", &status);
	if (gru_status_error(&status)) {
		fprintf(stderr, "%s", status.message);
		return;
	}

	ret->maestro_uri = gru_uri_parse("mqtt://localhost:1883/mpt/maestro", &status);
	if (gru_status_error(&status)) {
		fprintf(stderr, "%s", status.message);
		return;
	}

	ret->logdir = NULL;
	ret->parallel_count = 1;
	ret->count = 0;
	ret->log_level = INFO;
	ret->message_size = 32;
	ret->variable_size = false;
	ret->duration = gru_duration_new();
	ret->daemon = false;
	ret->probing = true;
	ret->throttle = 0;
	ret->iface = strdup("eth0");
	ret->probes = gru_split("net,bmic", ',', &status);
}

options_t *options_new() {
	options_t *ret = (options_t *) calloc(1, sizeof(options_t));

	if (!ret) {
		fprintf(stderr, "Not enough memory to allocate for options object\n");

		return NULL;
	}

	ret->logdir = NULL;
	options_set_defaults(ret);

	return ret;
}

void options_destroy(options_t **obj) {
	options_t *opt = (*obj);

	if (!opt) {
		return;
	}

	gru_split_clean(opt->probes);

	free(opt->iface);
	free(opt->logdir);
	free(opt);

	*obj = NULL;
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