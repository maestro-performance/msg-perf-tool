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
#include <MQTTClient.h>
#include "maestro_player.h"

static maestro_player_t *splayer;

static maestro_player_t *maestro_player_new() {
	maestro_player_t *ret = gru_alloc(sizeof(maestro_player_t), NULL);

	ret->mmsl = vmsl_init();

	return ret;
}

static void maestro_player_destroy(maestro_player_t **ptr, gru_status_t *status) {
	maestro_player_t *pl = *ptr;

	pl->mmsl.stop(pl->ctxt, status);
	pl->mmsl.destroy(pl->ctxt, status);

	gru_uri_cleanup(&pl->uri);

	vmslh_cleanup(&pl->handlers);

	gru_dealloc((void **) ptr);
}

static msg_content_data_t *wdata;
static char *wtopic = "/mpt/maestro";
static MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;

void maestro_abormal_disconnect_notice(void *ctxt, void *conn_opts, void *payload) {
	MQTTClient_connectOptions *opts = (MQTTClient_connectOptions *) conn_opts;

	maestro_note_t note = {0};

	maestro_note_payload_prepare(&note, NULL);

	maestro_note_set_type(&note, MAESTRO_TYPE_RESPONSE);

	maestro_note_set_cmd(&note, MAESTRO_NOTE_ABNORMAL_DISCONNECT);
	maestro_note_response_set_id(&note, splayer->player_info.id);
	maestro_note_response_set_name(&note, splayer->player_info.name);

	wdata = msg_content_data_new(MAESTRO_NOTE_SIZE, NULL);
	maestro_serialize_note(&note, wdata);

	opts->will = &wopts;
	opts->will->payload.data = wdata->data;
	opts->will->payload.len = (int) wdata->size;
	opts->will->topicName = wtopic;
	opts->will->qos = 1;
	opts->will->retained = 0;
}


static bool maestro_player_connect(maestro_player_t *player, gru_status_t *status) {
	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER,
		.statistics = MSG_STAT_NONE,
	};

	opt.conn_info.id = player->player_info.id;
	opt.uri = player->uri;

	player->handlers = vmslh_new(status);
	vmslh_add(player->handlers.before_connect, maestro_abormal_disconnect_notice, NULL, status);

	player->ctxt = player->mmsl.init(opt, &player->handlers, status);

	if (!player->ctxt) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Failed to initialize maestro connection: %s", status->message);
		return false;
	}

	vmsl_stat_t ret = player->mmsl.start(player->ctxt, status);
	if (vmsl_stat_error(ret)) {
		return false;
	}

	player->mmsl.subscribe(player->ctxt, &player->sheet->location, status);

	return true;
}

static void *maestro_player_run(void *player) {
	gru_status_t status = gru_status_new();
	maestro_player_t *maestro_player = (maestro_player_t *) player;
	logger_t logger = gru_logger_get();

	msg_content_data_t *mdata = msg_content_data_new(MAESTRO_NOTE_SIZE, &status);
	if (!gru_status_success(&status)) {
		return NULL;
	}

	logger(INFO, "Maestro player is running");
	while (!maestro_player->cancel) {
		msg_content_data_reset(mdata);
		vmsl_stat_t rstat =
			maestro_player->mmsl.receive(maestro_player->ctxt, mdata, &status);

		if (unlikely(vmsl_stat_error(rstat))) {
			logger(DEBUG, "Error receiving maestro data");
		} else {
			if (!(rstat & VMSL_NO_DATA)) {
				msg_content_data_t resp = {0};
				maestro_sheet_play(maestro_player->sheet,
					&maestro_player->player_info,
					mdata,
					&resp,
					&status);
				if (!gru_status_success(&status)) {
					logger(WARNING, "Maestro request failed: %s", status.message);
				}

				if (resp.data != NULL) {
					gru_uri_set_path(&maestro_player->ctxt->msg_opts.uri, "/mpt/maestro");

					vmsl_stat_t ret =
						maestro_player->mmsl.send(maestro_player->ctxt, &resp, &status);
					if (ret != VMSL_SUCCESS) {
						logger(
							ERROR, "Unable to write maestro reply: %s", status.message);
					}

					msg_content_data_release(&resp);
				}
			}
		}

		usleep(MPT_MAESTRO_IDLE_TIME);
	}

	logger(INFO, "Maestro player is terminating");
	msg_content_data_destroy(&mdata);


	return NULL;
}

bool maestro_player_start(const options_t *options,
	maestro_sheet_t *sheet,
	gru_status_t *status) {
	logger_t logger = gru_logger_get();

	if (splayer) {
		gru_status_set(status, GRU_FAILURE, "Maestro player is already initialized");

		return false;
	}

	splayer = maestro_player_new();
	splayer->sheet = sheet;

	logger(INFO, "Connecting to maestro host %s", options->maestro_uri.host);
	splayer->uri = gru_uri_clone(options->maestro_uri, status);
	if (!gru_status_success(status)) {
		return false;
	}

	if (!vmsl_assign_by_url(&splayer->uri, &splayer->mmsl)) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to assign a VMSL for the maestro player");

		return false;
	}

	splayer->player_info.name = options->name;
	msg_conn_info_gen_id_char(&splayer->player_info.id);
	if (!splayer->player_info.id) {
		logger(ERROR, "Unable to generate a player ID");

		goto err_exit;
	}

	if (!maestro_player_connect(splayer, status)) {
		logger(ERROR,
			"Unable to connect to maestro broker at %s: %s",
			   splayer->uri.host,
			status->message);

		goto err_exit;
	}

	logger(DEBUG, "Creating maestro player thread");
	int ret =
		pthread_create(&splayer->thread, NULL, maestro_player_run, splayer);
	if (ret != 0) {
		logger(ERROR, "Unable to create maestro player thread");

		goto err_exit;
	}

	return true;

err_exit:

	gru_uri_cleanup(&splayer->uri);
	return false;
}



bool maestro_player_stop(maestro_sheet_t *sheet, gru_status_t *status) {
	if (!splayer) {
		return true;
	}
	void *res;

	splayer->cancel = true;

	if (splayer->thread != 0) {
		pthread_join(splayer->thread, &res);
	}
	else {
		logger_t logger = gru_logger_get();

		logger(WARNING, "Invalid maestro player thread PID");
	}

	maestro_player_destroy(&splayer, status);
	splayer = NULL;

	return true;
}

const maestro_player_t *maestro_player() {
	return splayer;
}
