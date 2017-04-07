/**
 Copyright 2016 Otavio Rodolfo Piske

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
#include "config.h"

static char *log_level_str[7] = {
	"trace",
	"debug",
	"info",
	"stat",
	"warning",
	"error",
	"fatal",
};

void initialize_options(void *data) {
	options_t *options = (options_t *) data;

	options_set_defaults(options);
}

void save_options(FILE *file, void *data) {

	options_t *options = (options_t *) data;
	gru_status_t status = gru_status_new();

	char *tmp_url = gru_uri_simple_format(&options->uri, &status);
	gru_config_write_string("broker.url", file, tmp_url);
	gru_dealloc_string(&tmp_url);

	tmp_url = gru_uri_simple_format(&options->maestro_uri, &status);
	gru_config_write_string("maestro.url", file, tmp_url);
	gru_dealloc_string(&tmp_url);

	gru_config_write_ulong("message.count", file, options->count);
	gru_config_write_uint("message.throttle", file, options->throttle);
	gru_config_write_uint("message.size", file, options->message_size);

	gru_config_write_ushort("parallel.count", file, options->parallel_count);
	gru_config_write_ulong(
		"test.duration", file, gru_duration_minutes(options->duration, NULL));

	gru_config_write_string("log.level", file, log_level_str[options->log_level]);
	gru_config_write_string("log.dir", file, options->logdir);

	if (options->probing) {
		gru_config_write_string("probes.enabled", file, "true");
	}
	else {
		gru_config_write_string("probes.enabled", file, "false");
	}

	char *probes = gru_str_serialize(options->probes, ',', &status);
	gru_config_write_string("probes.list", file, probes);
	gru_dealloc_string(&probes);

	fflush(file);
}

void read_options(FILE *file, void *data) {
	options_t *options = (options_t *) data;

	gru_status_t status = gru_status_new();
	char tmp_url[4096] = {0};
	gru_config_read_string("broker.url", file, tmp_url);
	options->uri = gru_uri_parse(tmp_url, &status);
	if (gru_status_error(&status)) {
		fprintf(stderr, "%s\n", status.message);
		return;
	}

	gru_config_read_string("maestro.url", file, tmp_url);
	options->maestro_uri = gru_uri_parse(tmp_url, &status);
	if (gru_status_error(&status)) {
		fprintf(stderr, "%s\n", status.message);
		return;
	}

	gru_config_read_ulong("message.count", file, &options->count);

	gru_config_read_ulong("message.count", file, &options->count);
	gru_config_read_uint("message.throttle", file, &options->throttle);
	gru_config_read_ulong("message.size", file, &options->message_size);

	gru_config_read_ushort("parallel.count", file, &options->parallel_count);

	uint64_t duration_minutes;
	gru_config_read_ulong("test.duration", file, &duration_minutes);
	options->duration = gru_duration_from_minutes(duration_minutes);

	char log_level_s[OPT_MAX_STR_SIZE] = {0};
	gru_config_read_string("log.level", file, log_level_s);
	options->log_level = gru_logger_get_level(log_level_s);

	gru_config_read_string("log.dir", file, options->logdir);

	char probes_enabled_setting[10] = {0};
	gru_config_read_string("probes.enabled", file, probes_enabled_setting);
	if (strlen(probes_enabled_setting) == 0 || strcmp(probes_enabled_setting, "true")) {
		options->probing = true;
	}
	else {
		options->probing = false;
	}

	char probes_list[4096] = {0};
	gru_config_read_string("probes.list", file, probes_list);
	options->probes = gru_split(probes_list, ',', &status);
	if (gru_status_error(&status)) {
		fprintf(stderr, "%s\n", status.message);
		return;
	}
}

void config_init(
	options_t *options, const char *dir, const char *filename, gru_status_t *status) {
	gru_payload_t *payload =
		gru_payload_init(initialize_options, save_options, read_options, options, status);

	if (!payload) {
		fprintf(stderr, "Unable to initialize the payload: %s\n", status->message);

		gru_payload_destroy(&payload);
		return;
	}

	if (!gru_path_exists(dir, status)) {
		if (gru_status_error(status)) {
			return;
		}

		gru_path_mkdirs(dir, status);
	}

	options->config = gru_config_init(dir, filename, payload, status);
	if (!options->config) {
		if (gru_status_error(status)) {
			fprintf(
				stderr, "Unable to initialize the configuration: %s\n", status->message);
		}

		gru_payload_destroy(&payload);
		return;
	}

	gru_payload_destroy(&payload);
}