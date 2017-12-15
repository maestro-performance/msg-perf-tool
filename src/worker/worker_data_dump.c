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
#include <common/gru_variant.h>
#include "worker_data_dump.h"

struct worker_dump_wrapper_t {
  worker_options_t *options;
  vmsl_info_t *info;
};

static void worker_dump_data(FILE *file, void *data) {
	struct worker_dump_wrapper_t *wrapper = (struct worker_dump_wrapper_t *) data;

	gru_status_t status = gru_status_new();
	char *uri_str = gru_uri_simple_format(&wrapper->options->uri, &status);
	gru_config_write_string("brokerUri", file, uri_str);
	gru_dealloc_string(&uri_str);

	if (wrapper->options->duration_type == MESSAGE_COUNT) {
		gru_config_write_string("durationType", file, "count");
		gru_config_write_ulong("duration", file, wrapper->options->duration.count);
	}
	else {
		uint64_t duration = gru_duration_seconds(wrapper->options->duration.time);
		gru_config_write_string("durationType", file, "time");
		gru_config_write_ulong("duration", file, duration);
	}

	gru_config_write_short("parallelCount", file, wrapper->options->parallel_count);
	gru_config_write_ulong("messageSize", file, wrapper->options->message_size);
	gru_config_write_short("variableSize", file, wrapper->options->variable_size ? 1 : 0);
	gru_config_write_uint("rate", file, wrapper->options->rate);

	if (wrapper->options->condition_type == MPT_COND_LATENCY) {
		gru_config_write_long("fcl", file, wrapper->options->condition.latency);
	}

	// if TCP, then it's probably using the java backend and the test is
	// using JMS. In this case, the protocol comes as part of the URL
	if (strcmp(wrapper->options->uri.scheme, "tcp")) {
		gru_uri_t uri = wrapper->options->uri;

		if (!uri.query) {
			return;
		}

		gru_node_t *node = uri.query->root;

		while (node) {
			gru_keypair_t *kp = (gru_keypair_t *) node->data;
			if (gru_keypair_key_equals(kp, "protocol")) {
				gru_config_write_string("protocol", file, kp->pair->variant.string);
				break;
			}

			node = node->next;
		}

		gru_config_write_string("apiName", file, "JMS");
		gru_config_write_string("apiVersion", file, "1.1 (best guess)");

	}
	else {
		gru_config_write_string("protocol", file, wrapper->options->uri.scheme);

		if (wrapper->info) {
			gru_config_write_string("apiName", file, wrapper->info->api_name);
			gru_config_write_string("apiVersion", file, wrapper->info->api_version);
		}
	}


	fflush(file);
}

bool worker_dump(const char *dir, worker_options_t *options, vmsl_info_t *info, gru_status_t *status) {
	gru_config_t *config = gru_config_new(dir, "test.properties", status);
	gru_alloc_check(config, false);

	struct worker_dump_wrapper_t wrapper = {
		.options = options,
		.info = info,
	};

	gru_payload_t *payload = gru_payload_init(NULL, worker_dump_data, NULL, &wrapper, status);
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