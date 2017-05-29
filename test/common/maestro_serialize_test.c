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

#include "msg_content_data.h"
#include "maestro/maestro_note.h"
#include "maestro/maestro_serialize.h"

#include <msgpack.h>

static void sample_stats(maestro_note_t *response) {
	maestro_note_set_type(response, MAESTRO_TYPE_RESPONSE);
	maestro_note_set_cmd(response, MAESTRO_NOTE_STATS);
	maestro_note_stats_set_id(response, "07a549ac-d490-4cce-8b27-551cfe8c991f");
	maestro_note_stats_set_name(response, "test@unit.test");

	maestro_note_stats_set_child_count(response, 6);
	maestro_note_stats_set_role(response, "receiver");
	maestro_note_stats_set_roleinfo(response, "perf");
	maestro_note_stats_set_stat_type(response, 'R');

	gru_timestamp_t now = gru_time_now();
	char *formatted_ts = gru_time_write_str(&now);

	maestro_note_stats_set_perf_ts(response, formatted_ts);
	gru_dealloc_string(&formatted_ts);

	maestro_note_stats_set_perf_count(response, 100);
	maestro_note_stats_set_perf_rate(response, 10);
	maestro_note_stats_set_perf_latency(response, 11.2);
}

static int maestro_serialize_stats_test() {
	maestro_note_t sample = {0};
	gru_status_t status = gru_status_new();

	if (!maestro_note_payload_prepare(&sample, &status)) {
		fprintf(stderr, "%s\n", status.message);

		return EXIT_FAILURE;
	}

	sample_stats(&sample);

	msg_content_data_t content = {0};
	msg_content_data_init(&content, MAESTRO_NOTE_SIZE, &status);
	if (!gru_status_success(&status)) {
		fprintf(stderr, "%s\n", status.message);

		return EXIT_FAILURE;
	}

	maestro_serialize_note(&sample, &content);

	printf("Wrote %d bytes (expected %d)\n", content.size, MAESTRO_NOTE_SIZE);
	printf("Data: %-*s|||\n", MAESTRO_NOTE_SIZE, (char *) content.data);

	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&sample);
	return EXIT_SUCCESS;
}


static void unpack(const void *ptr, int size, maestro_note_t *note) {

}

static int maestro_serialize_cmd_ok_test() {
	gru_status_t status = gru_status_new();
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_RESPONSE);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_OK);

	msg_content_data_t content = {0};
	msg_content_data_init(&content, sizeof(maestro_note_t), &status);

	maestro_serialize_note(&note, &content);

	maestro_note_t ok = {0};
	if (!maestro_deserialize_note(&content, &ok, &status)) {
		fprintf(stderr, "Failed to deserialize note: %s\n", status.message);
		goto err_exit;
	}

	if (ok.type != MAESTRO_TYPE_RESPONSE) {
		fprintf(stderr, "Invalid type: %c\n", ok.type);
		goto err_exit;
	}

	msg_content_data_release(&content);
	return EXIT_SUCCESS;

	err_exit:
	msg_content_data_release(&content);
	return EXIT_FAILURE;
}

int main(int argc, char **argv) {
	return maestro_serialize_cmd_ok_test();
}