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

maestro_cmd_ctxt_t *maestro_cmd_init_ctxt(const options_t *options, gru_status_t *status) {
	maestro_cmd_ctxt_t *ret = gru_alloc(sizeof(maestro_cmd_ctxt_t), status);
	gru_alloc_check(ret, NULL);

	ret->vmsl = vmsl_init();

	if (!vmsl_assign_by_url(&options->maestro_uri, &ret->vmsl)) {
		return NULL;
	}

	return ret;
}

static int maestro_cmd_connect(maestro_cmd_ctxt_t *cmd_ctxt, gru_uri_t uri, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_BOTH, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_NONE,
	};

	opt.uri = uri;
	// gru_uri_set_path(&opt.uri, "/mpt/receiver");

	cmd_ctxt->msg_ctxt = cmd_ctxt->vmsl.init(NULL, opt, NULL, status);

	if (!cmd_ctxt->msg_ctxt) {
		logger(ERROR, "Failed to initialize maestro connection: %s", status->message);
		return 1;
	}

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

	// Send to /mpt/receiver
	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, "/mpt/receiver");
	
	msg_content_data_t req = {0};

	msg_content_data_init(&req, 3, NULL);
	req.data = strdup("001");
	req.size = 3;
	
	cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, &req, status);
	ret = maestro_cmd_disconnect(cmd_ctxt, status);
	if (ret != 0) {
		fprintf(stderr, "Warning error during disconnect");
	}

	return 0;
}

