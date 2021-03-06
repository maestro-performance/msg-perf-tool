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
#include "maestro/maestro_deserialize.h"

static bool maestro_serialize_cmd_ping_request_test() {
	gru_status_t status = gru_status_new();
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_REQUEST);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_PING);
	if (!maestro_note_payload_prepare(&note, &status)) {
		return false;
	}

	msg_content_data_t content = {0};
	msg_content_data_init(&content, sizeof(maestro_note_t), &status);

	gru_timestamp_t ts = gru_time_now();
	maestro_note_ping_set_ts(&note, ts);

	maestro_serialize_note(&note, &content);
	maestro_note_payload_cleanup(&note);

	maestro_note_t deserialized = {0};
	if (!maestro_deserialize_note(&content, &deserialized, &status)) {
		fprintf(stderr, "Failed to deserialize note: %s\n", status.message);
		goto err_exit;
	}

	if (deserialized.type != MAESTRO_TYPE_REQUEST) {
		fprintf(stderr, "Invalid type: %c\n", deserialized.type);
		goto err_exit;
	}

	if (deserialized.command != MAESTRO_NOTE_PING) {
		fprintf(stderr, "Unexpected maestro command: %"PRIu32"\n", deserialized.command);
		goto err_exit;
	}

	if (deserialized.payload->request.ping.sec != ts.tv_sec) {
		fprintf(stderr, "Unexpected ping timestamp sec: %"PRIu64" != %"PRIu64"\n",
			deserialized.payload->request.ping.sec, ts.tv_sec);
		goto err_exit;
	}

	if (deserialized.payload->request.ping.usec != ts.tv_usec) {
		fprintf(stderr, "Unexpected ping timestamp usec: %"PRIu64" != %"PRIu64"\n",
				deserialized.payload->request.ping.usec, ts.tv_usec);
		goto err_exit;
	}

	msg_content_data_release(&content);
  	maestro_note_payload_cleanup(&deserialized);
	return true;

	err_exit:
	msg_content_data_release(&content);
  	maestro_note_payload_cleanup(&deserialized);
	return false;
}


static bool maestro_serialize_cmd_ping_response_test() {
	gru_status_t status = gru_status_new();
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_RESPONSE);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_PING);
	if (!maestro_note_payload_prepare(&note, &status)) {
		return false;
	}

	msg_content_data_t content = {0};
	msg_content_data_init(&content, sizeof(maestro_note_t), &status);

	maestro_note_response_set_id(&note, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3");
	maestro_note_response_set_name(&note, "test@localhost.localdomain");
  	maestro_note_ping_set_elapsed(&note, 30);

	maestro_serialize_note(&note, &content);
	maestro_note_payload_cleanup(&note);

	maestro_note_t deserialized = {0};
	if (!maestro_deserialize_note(&content, &deserialized, &status)) {
		fprintf(stderr, "Failed to deserialize note: %s\n", status.message);
		goto err_exit;
	}

	if (deserialized.type != MAESTRO_TYPE_RESPONSE) {
		fprintf(stderr, "Invalid type: %c\n", deserialized.type);
		goto err_exit;
	}

	if (deserialized.command != MAESTRO_NOTE_PING) {
		fprintf(
			stderr, "Unexpected maestro command: %" PRIu32 "\n", deserialized.command);
		goto err_exit;
	}

	if (strcmp(deserialized.payload->response.id, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3") != 0) {
		fprintf(stderr, "Unexpected ping id: %s != %s\n",
				deserialized.payload->response.id, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3");
		goto err_exit;
	}

	if (strcmp(deserialized.payload->response.name, "test@localhost.localdomain") != 0) {
	  	fprintf(stderr, "Unexpected ping name: %s != %s\n",
			  deserialized.payload->response.name, "test@localhost.localdomain");
	  	goto err_exit;
	}

  	if (deserialized.payload->response.body.ping.elapsed != 30) {
		fprintf(stderr, "Unexpected ping elapsed time: %"PRId64" != %"PRId32"\n",
				deserialized.payload->response.body.ping.elapsed, 30);
		goto err_exit;
	}


  	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&note);
	return true;

	err_exit:

	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&note);
	return false;
}


static bool maestro_serialize_cmd_ok_test() {
	gru_status_t status = gru_status_new();
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_RESPONSE);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_OK);

	if (!maestro_note_payload_prepare(&note, &status)) {
		return false;
	}
	maestro_note_response_set_id(&note, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3");
	maestro_note_response_set_name(&note, "test@localhost.localdomain");

	msg_content_data_t content = {0};
	msg_content_data_init(&content, sizeof(maestro_note_t), &status);

	maestro_serialize_note(&note, &content);
	maestro_note_payload_cleanup(&note);

	maestro_note_t ok = {0};
	if (!maestro_deserialize_note(&content, &ok, &status)) {
		fprintf(stderr, "Failed to deserialize note: %s\n", status.message);
		goto err_exit;
	}

	if (ok.type != MAESTRO_TYPE_RESPONSE) {
		fprintf(stderr, "Invalid type: %c\n", ok.type);
		goto err_exit;
	}

	if (ok.command != MAESTRO_NOTE_OK) {
		fprintf(stderr, "Unexpected maestro command: %"PRIu32"\n", ok.command);
		goto err_exit;
	}

	if (strcmp(ok.payload->response.id, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3") != 0) {
		fprintf(stderr, "Unexpected stat id: %s != %s\n",
				ok.payload->response.id, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3");
		goto err_exit;
	}

	if (strcmp(ok.payload->response.name, "test@localhost.localdomain") != 0) {
		fprintf(stderr, "Unexpected stat name: %s != %s\n",
				ok.payload->response.name, "test@localhost.localdomain");
		goto err_exit;
	}

	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&ok);
	return true;

	err_exit:
	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&ok);
	return false;
}


static bool maestro_serialize_cmd_set_opt_test() {
	gru_status_t status = gru_status_new();
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_REQUEST);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_SET);
	maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_BROKER, "amqp://test.host:5672/queue");

	msg_content_data_t content = {0};
	msg_content_data_init(&content, sizeof(maestro_note_t), &status);

	maestro_serialize_note(&note, &content);

	maestro_note_t deserialized = {0};
	if (!maestro_deserialize_note(&content, &deserialized, &status)) {
		fprintf(stderr, "Failed to deserialize note: %s\n", status.message);
		goto err_exit;
	}

	if (deserialized.type != MAESTRO_TYPE_REQUEST) {
		fprintf(stderr, "Invalid type: %c\n", deserialized.type);
		goto err_exit;
	}

	if (deserialized.command != MAESTRO_NOTE_SET) {
		fprintf(stderr, "Unexpected maestro command: %"PRIu32"\n", deserialized.command);
		goto err_exit;
	}

	if (deserialized.payload->request.set.opt != MAESTRO_NOTE_OPT_SET_BROKER) {
		fprintf(stderr, "Unexpected set command: %li\n", deserialized.payload->request.set.opt);
		goto err_exit;
	}

	if (strcmp(deserialized.payload->request.set.value, "amqp://test.host:5672/queue") != 0) {
		fprintf(stderr, "Unexpected set value: %s\n", deserialized.payload->request.set.value);
		goto err_exit;
	}

	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&deserialized);
	maestro_note_payload_cleanup(&note);
	return true;

	err_exit:
	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&deserialized);
	maestro_note_payload_cleanup(&note);
	return false;
}


static bool maestro_serialize_cmd_stats_response_test() {
	gru_status_t status = gru_status_new();
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_RESPONSE);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_STATS);
	if (!maestro_note_payload_prepare(&note, &status)) {
		return false;
	}

	msg_content_data_t content = {0};
	msg_content_data_init(&content, sizeof(maestro_note_t), &status);

	maestro_note_response_set_id(&note, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3");
	maestro_note_response_set_name(&note, "test@localhost.localdomain");
	maestro_note_stats_set_child_count(&note, 6);

	maestro_note_stats_set_role(&note, "receiver");
	maestro_note_stats_set_roleinfo(&note, "perf");
	maestro_note_stats_set_stat_type(&note, 'R');

	gru_timestamp_t now = gru_time_now();
	char *formatted_ts = gru_time_write_str(&now);

	maestro_note_stats_set_perf_ts(&note, formatted_ts);
	gru_dealloc_string(&formatted_ts);

	maestro_note_stats_set_perf_count(&note, 10000);
	maestro_note_stats_set_perf_rate(&note, (535 / 6));
	maestro_note_stats_set_perf_latency(&note, 45.6);

	maestro_serialize_note(&note, &content);
	maestro_note_payload_cleanup(&note);

	maestro_note_t deserialized = {0};
	if (!maestro_deserialize_note(&content, &deserialized, &status)) {
		fprintf(stderr, "Failed to deserialize note: %s\n", status.message);
		goto err_exit;
	}

	if (deserialized.type != MAESTRO_TYPE_RESPONSE) {
		fprintf(stderr, "Invalid type: %c\n", deserialized.type);
		goto err_exit;
	}

	if (deserialized.command != MAESTRO_NOTE_STATS) {
		fprintf(
			stderr, "Unexpected maestro command: %" PRIu32 "\n", deserialized.command);
		goto err_exit;
	}

	if (strcmp(deserialized.payload->response.id, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3") != 0) {
		fprintf(stderr, "Unexpected stat id: %s != %s\n",
				deserialized.payload->response.id, "71a1b3d2-45e2-11e7-9de0-68f72834b1b3");
		goto err_exit;
	}

	if (strcmp(deserialized.payload->response.name, "test@localhost.localdomain") != 0) {
		fprintf(stderr, "Unexpected stat name: %s != %s\n",
				deserialized.payload->response.name, "test@localhost.localdomain");
		goto err_exit;
	}

	if (deserialized.payload->response.body.stats.child_count != 6) {
		fprintf(stderr, "Unexpected stat child count: %"PRIu32" != %"PRIu32"\n",
				deserialized.payload->response.body.stats.child_count, 6);
		goto err_exit;
	}

	if (strcmp(deserialized.payload->response.body.stats.role, "receiver") != 0) {
		fprintf(stderr, "Unexpected stat role: %s != %s\n",
				deserialized.payload->response.body.stats.role, "receiver");
		goto err_exit;
	}

	if (strcmp(deserialized.payload->response.body.stats.roleinfo, "perf") != 0) {
		fprintf(stderr, "Unexpected stat role info: %s != %s\n",
				deserialized.payload->response.body.stats.roleinfo, "perf");
		goto err_exit;
	}

	if (deserialized.payload->response.body.stats.stats.perf.count != 10000) {
		fprintf(stderr, "Unexpected stat message count: %"PRIu64" != %"PRIu32"\n",
				deserialized.payload->response.body.stats.stats.perf.count, 10000);
		goto err_exit;
	}

	if (deserialized.payload->response.body.stats.stats.perf.rate != (535 / 6)) {
		fprintf(stderr, "Unexpected stat child count: %f != %f\n",
				deserialized.payload->response.body.stats.stats.perf.rate, (double) (535 / 6));
		goto err_exit;
	}

	if (deserialized.payload->response.body.stats.stats.perf.latency != 45.6) {
		fprintf(stderr, "Unexpected stat child count: %f != %f\n",
				deserialized.payload->response.body.stats.stats.perf.latency, 45.6);
		goto err_exit;
	}

	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&deserialized);
	return true;

	err_exit:
	msg_content_data_release(&content);
	maestro_note_payload_cleanup(&deserialized);
	return false;
}


int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Missing test case name\n");

		return EXIT_FAILURE;
	}

	if (strncmp(argv[1], "ok", 2) == 0) {
		if (!maestro_serialize_cmd_ok_test()) {
			return EXIT_FAILURE;
		}
	} else if (strncmp(argv[1], "set-opt", 7) == 0) {
		if (!maestro_serialize_cmd_set_opt_test()) {
			return EXIT_FAILURE;
		}
	} else if (strncmp(argv[1], "ping-request", 12) == 0) {
		if (!maestro_serialize_cmd_ping_request_test()) {
			return EXIT_FAILURE;
		}
	} else if (strncmp(argv[1], "ping-response", 13) == 0) {
		if (!maestro_serialize_cmd_ping_response_test()) {
			return EXIT_FAILURE;
		}
	} else if (strncmp(argv[1], "stats-response", 14) == 0) {
		if (!maestro_serialize_cmd_stats_response_test()) {
			return EXIT_FAILURE;
		}
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}