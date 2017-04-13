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

#include "maestro_command.h"


static int maestro_cmd_connect(maestro_cmd_ctxt_t *cmd_ctxt, gru_uri_t uri, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_NONE,
	};

	msg_conn_info_gen_id(&opt.conn_info);
	opt.uri = uri;
	
	cmd_ctxt->msg_ctxt = cmd_ctxt->vmsl.init(NULL, opt, NULL, status);

	if (!cmd_ctxt->msg_ctxt) {
		logger(ERROR, "Failed to initialize maestro connection: %s", status->message);
		return 1;
	}

	// gru_uri_set_path(&opt.uri, "/mpt/maestro");
	// cmd_ctxt->vmsl.subscribe(cmd_ctxt->msg_ctxt, NULL, status);

	return 0;
}


static int maestro_cmd_disconnect(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	cmd_ctxt->vmsl.stop(cmd_ctxt->msg_ctxt, status);
	return 0;
}



int maestro_cmd_start_receiver(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	const options_t *options = get_options_object();

	int ret = maestro_cmd_connect(cmd_ctxt, options->maestro_uri, status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, "/mpt/receiver");
	
	msg_content_data_t req = {0};

	maestro_note_serialize(&req, maestro_request(MAESTRO_NOTE_START));

	vmsl_stat_t rstat = cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, &req, status);
	if (rstat != VMSL_SUCCESS) {
		fprintf(stderr, "Failed to send command");
	}

	ret = maestro_cmd_disconnect(cmd_ctxt, status);
	if (ret != 0) {
		fprintf(stderr, "Warning error during disconnect");
	}

	return 0;
}



int maestro_cmd_collect(maestro_cmd_ctxt_t *cmd_ctxt, int queue, gru_status_t *status) {
	int ret = 0;
	while (ret == 0) {
		char buf[MAESTRO_NOTE_SIZE] = {0};

		ret = msgrcv(queue, &buf, sizeof(buf), 0, IPC_NOWAIT);
		if (ret < 0) {
			if (errno == ENOMSG) { 
				fprintf(stdout, "No data to collect\n");
			}
			else {
				fprintf(stdout, "Failed to read from the local forward queue\n");
				return 1;
			}
		}
		else {
			fprintf(stdout, "Just read: %s\n", buf);
		}
	}

	return 0;
}