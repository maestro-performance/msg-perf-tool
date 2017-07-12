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
#include "maestro_notify.h"

void maestro_notify_test_failed(gru_status_t *status) {
	logger_t logger = gru_logger_get();

	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_NOTIFICATION);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_NOTIFY_FAIL);

	const maestro_player_t *player = maestro_player();

	maestro_note_response_set_id(&note, player->player_info.id);
	maestro_note_response_set_name(&note, player->player_info.name);
	maestro_note_payload_prepare(&note, status);

	gru_uri_set_path(&player->ctxt->msg_opts.uri, MAESTRO_NOTIFICATIONS);

	msg_content_data_t data = {0};
	msg_content_data_init(&data, MAESTRO_NOTE_SIZE, status);

	maestro_serialize_note(&note, &data);

	vmsl_stat_t ret = player->mmsl.send(player->ctxt, &data, status);
	if (ret != VMSL_SUCCESS) {
		logger(
			ERROR, "Unable to write maestro notification: %s", status->message);
	}

	msg_content_data_release(&data);
	maestro_note_payload_cleanup(&note);
}
