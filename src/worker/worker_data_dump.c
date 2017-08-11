/**
 *    Copyright 2017 Otavio Rodolfo Piske
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "worker_data_dump.h"
#include "worker_options.h"

static void worker_dump_data(FILE *file, void *data) {
	worker_options_t *options = (worker_options_t *) data;

	gru_status_t status = gru_status_new();
	char *uri_str = gru_uri_simple_format(&options->uri, &status);
	gru_config_write_string("brokerUri", file, uri_str);
	gru_dealloc_string(&uri_str);

	if (options->duration_type == MESSAGE_COUNT) {
		gru_config_write_string("durationType", file, "count");
		gru_config_write_ulong("duration", file, options->duration.count);
	}
	else {
		uint64_t duration = gru_duration_seconds(options->duration.time);
		gru_config_write_string("durationType", file, "time");
		gru_config_write_ulong("duration", file, duration);
	}

	gru_config_write_short("parallelCount", file, options->parallel_count);
	gru_config_write_ulong("messageSize", file, options->message_size);
	gru_config_write_short("variableSize", file, options->variable_size ? 1 : 0);
	gru_config_write_uint("rate", file, options->rate);

	if (options->condition_type == MPT_COND_LATENCY) {
		gru_config_write_long("fcl", file, options->condition.latency);
	}

	fflush(file);
}

bool worker_dump(const char *dir, worker_options_t *options, gru_status_t *status) {
	gru_config_t *config = gru_config_new(dir, "test.properties", status);
	gru_alloc_check(config, false);

	gru_payload_t *payload = gru_payload_init(NULL, worker_dump_data, NULL, options, status);
	if (!payload) {
		gru_config_destroy(&config);
		return false;
	}

	if (!gru_config_init_for_dump(config, payload, status)) {

		gru_payload_destroy(&payload);
		gru_config_destroy(&config);
		return false;
	}

	gru_payload_destroy(&payload);
	gru_config_destroy(&config);

	return true;
}