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
#include "maestro_cmd_ctxt.h"

maestro_cmd_ctxt_t *maestro_cmd_ctxt_new(const gru_uri_t *uri, gru_status_t *status) {
	maestro_cmd_ctxt_t *ret = gru_alloc(sizeof(maestro_cmd_ctxt_t), status);
	gru_alloc_check(ret, NULL);

	ret->vmsl = vmsl_init();

	if (!vmsl_assign_by_url(uri, &ret->vmsl)) {
		return NULL;
	}

	ret->queue = create_foward_queue(status);
	if (ret->queue < 0) {
		gru_status_set(status, GRU_FAILURE, "Unable to initialize forward queue: %s\n",
			status->message);

		gru_dealloc((void **) &ret);

		return NULL;
	}

	return ret;
}

void maestro_cmd_ctxt_destroy(maestro_cmd_ctxt_t **ptr) {
	logger_t  logger = gru_logger_get();
	maestro_cmd_ctxt_t *ctxt = *ptr;


	logger(INFO, "Destroying context");
	if (!ctxt) {
		return;
	}

	if (ctxt->msg_ctxt) {
		msg_conn_info_cleanup(&ctxt->msg_ctxt->msg_opts.conn_info);

		gru_status_t tmp = gru_status_new();

		ctxt->vmsl.stop(ctxt->msg_ctxt, &tmp);
		ctxt->vmsl.destroy(ctxt->msg_ctxt, &tmp);
	}

	gru_dealloc((void **) ptr);
}

bool maestro_cmd_ctxt_start(maestro_cmd_ctxt_t *cmd_ctxt, msg_opt_t opt, gru_status_t *status) {
	logger_t  logger = gru_logger_get();

	cmd_ctxt->msg_ctxt = cmd_ctxt->vmsl.init(opt, NULL, status);

	if (!cmd_ctxt->msg_ctxt) {
		logger(ERROR, "Failed to initialize maestro connection: %s", status->message);
		return false;
	}

	vmsl_stat_t sstat = cmd_ctxt->vmsl.start(cmd_ctxt->msg_ctxt, status);
	if (vmsl_stat_error(sstat)) {
		return false;
	}

	return true;
}


void maestro_cmd_ctxt_stop(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	logger_t  logger = gru_logger_get();

	logger(INFO, "Stopping context");
	cmd_ctxt->vmsl.stop(cmd_ctxt->msg_ctxt, status);

	gru_uri_cleanup(&cmd_ctxt->msg_ctxt->msg_opts.uri);
	msg_conn_info_cleanup(&cmd_ctxt->msg_ctxt->msg_opts.conn_info);

	cmd_ctxt->vmsl.destroy(cmd_ctxt->msg_ctxt, status);
	cmd_ctxt->msg_ctxt = NULL;
}


bool maestro_cmd_ctxt_forwarder(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	logger_t  logger = gru_logger_get();

	logger(DEBUG, "Subscribing to the Maestro topic for command forwarding");
	vmsl_stat_t rstat = cmd_ctxt->vmsl.subscribe(cmd_ctxt->msg_ctxt, NULL, status);
	if (vmsl_stat_error(rstat)) {
		gru_status_set(status, GRU_FAILURE, "Unable to subscribe to maestro broker");

		maestro_cmd_ctxt_destroy(&cmd_ctxt);
		return false;
	}

	return true;
}


bool maestro_cmd_ctxt_send(maestro_cmd_ctxt_t *cmd_ctxt, msg_content_data_t *cmd, gru_status_t *status) {
	vmsl_stat_t rstat = cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, cmd, status);

	if (rstat != VMSL_SUCCESS) {
		return false;
	}

	return true;
}


