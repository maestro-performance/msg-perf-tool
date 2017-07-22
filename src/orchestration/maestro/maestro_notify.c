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
	logger(INFO, "Sending a test failure notification");

	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_NOTIFICATION);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_NOTIFY_FAIL);

	gru_status_t self_status = gru_status_new();
	maestro_note_payload_prepare(&note, &self_status);

	const maestro_player_t *player = maestro_player();

	maestro_note_response_set_id(&note, player->player_info.id);
	maestro_note_response_set_name(&note, player->player_info.name);
	maestro_note_notification_set_message(&note, status->message);

	gru_uri_set_path(&player->ctxt->msg_opts.uri, MAESTRO_NOTIFICATIONS);

	msg_content_data_t data = {0};
	msg_content_data_init(&data, MAESTRO_NOTE_SIZE, status);

	maestro_serialize_note(&note, &data);

	/**
	 * It temporarily overrides the retained flag for the message
	 * so that the notification message is received by clients
	 * connecting after it was sent. A typical scenario here is when
	 * a long performance test starts running on Friday and the user
	 * will only connect on Monday.
	 */
	gru_variant_t retained = GRU_VARIANT_BOOLEAN_INITIALIZER(true);

	vmslh_add(player->handlers.before_send, paho_set_retained, &retained, status);

	vmsl_stat_t ret = player->mmsl.send(player->ctxt, &data, status);
	if (ret != VMSL_SUCCESS) {
		logger(
			ERROR, "Unable to write maestro notification: %s", status->message);
	}

	vmslh_add(player->handlers.before_send, paho_set_retained,
			  PAHO_HANDLERS_DEFAULT_RETAINED_PL, status);

	msg_content_data_release(&data);
	maestro_note_payload_cleanup(&note);
	logger(INFO, "Sent a test failure notification");
}


void maestro_notify_test_successful(gru_status_t *status) {
	logger_t logger = gru_logger_get();
	logger(INFO, "Sending a test success notification");

	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_NOTIFICATION);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_NOTIFY_SUCCESS);

	gru_status_t self_status = gru_status_new();
	maestro_note_payload_prepare(&note, &self_status);

	const maestro_player_t *player = maestro_player();

	maestro_note_response_set_id(&note, player->player_info.id);
	maestro_note_response_set_name(&note, player->player_info.name);
	maestro_note_notification_set_message(&note, "Test completed successfully");

	gru_uri_set_path(&player->ctxt->msg_opts.uri, MAESTRO_NOTIFICATIONS);

	msg_content_data_t data = {0};
	msg_content_data_init(&data, MAESTRO_NOTE_SIZE, status);

	maestro_serialize_note(&note, &data);

	/**
	 * It temporarily overrides the retained flag for the message
	 * so that the notification message is received by clients
	 * connecting after it was sent. A typical scenario here is when
	 * a long performance test starts running on Friday and the user
	 * will only connect on Monday.
	 */
	gru_variant_t retained = GRU_VARIANT_BOOLEAN_INITIALIZER(true);

	vmslh_add(player->handlers.before_send, paho_set_retained, &retained, status);

	vmsl_stat_t ret = player->mmsl.send(player->ctxt, &data, status);
	if (ret != VMSL_SUCCESS) {
		logger(
			ERROR, "Unable to write maestro notification: %s", status->message);
	}

	vmslh_add(player->handlers.before_send, paho_set_retained,
			  PAHO_HANDLERS_DEFAULT_RETAINED_PL, status);

	msg_content_data_release(&data);
	maestro_note_payload_cleanup(&note);
	logger(INFO, "Sent a test succss notification");
}