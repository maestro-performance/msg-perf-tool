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
#include <network/gru_uri.h>

#include "proton-context.h"
#include "proton-wrapper.h"
#include "vmsl.h"

const int window = 10;
const char *url = NULL;

static inline bool failed(pn_messenger_t *messenger) {
	if (pn_messenger_errno(messenger)) {
		return true;
	}

	return false;
}

static inline proton_ctxt_t *proton_ctxt_cast(msg_ctxt_t *ctxt) {
	return (proton_ctxt_t *) ctxt->api_context;
}

msg_ctxt_t *proton_init(msg_opt_t opt, vmslh_handlers_t *handlers, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Initializing proton wrapper");

	msg_ctxt_t *msg_ctxt = msg_ctxt_init(status);
	if (!msg_ctxt) {
		return NULL;
	}

	proton_ctxt_t *proton_ctxt = proton_context_init(handlers);

	if (!proton_ctxt) {
		logger(FATAL, "Unable to initialize the proton context");

		goto err_exit;
	}

	proton_set_default_parameters(handlers, opt, status);
	proton_set_user_parameters(handlers, opt, status);

	proton_ctxt->messenger = pn_messenger(NULL);
	msg_ctxt->api_context = proton_ctxt;
	msg_ctxt->msg_opts = opt;

	return msg_ctxt;

err_exit:
	msg_ctxt_destroy(&msg_ctxt);
	return NULL;
}

vmsl_stat_t proton_start(msg_ctxt_t *ctxt, gru_status_t *status) {
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);
	logger_t logger = gru_logger_get();

	vmslh_run(proton_ctxt->handlers->before_connect, proton_ctxt, NULL);

	logger(DEBUG, "Initializing the proton messenger");
	int err = pn_messenger_start(proton_ctxt->messenger);
	if (err) {
		gru_status_set(status, GRU_FAILURE, "Unable to start the proton messenger");

		return VMSL_ERROR;
	}

	url = gru_uri_simple_format(&ctxt->msg_opts.uri, status);
	if (gru_status_error(status)) {
		return VMSL_ERROR;
	}

	vmslh_run(proton_ctxt->handlers->after_connect, proton_ctxt, NULL);

	return VMSL_SUCCESS;
}

void proton_stop(msg_ctxt_t *ctxt, gru_status_t *status) {
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	int ret = pn_messenger_stop(proton_ctxt->messenger);
	if (ret == PN_INPROGRESS) {
		bool stopped = false;

		for (int i = 0; i < 10; i++) {
			if (!pn_messenger_stopped(proton_ctxt->messenger)) {
				usleep(100000);
			} else {
				stopped = true;
			}
		}

		if (!stopped) {
			logger_t logger = gru_logger_get();

			logger(WARNING, "Proton did not stop within the required wait time");
		}
	}
}

void proton_destroy(msg_ctxt_t *ctxt, gru_status_t *status) {
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	pn_messenger_free(proton_ctxt->messenger);
	proton_context_destroy(&proton_ctxt);

	if (url) {
		gru_dealloc_const_string(&url);
	}

	msg_ctxt_destroy(&ctxt);

	proton_param_cleanup();
}

static pn_timestamp_t proton_now(gru_status_t *status) {
	struct timeval now;

	if (gettimeofday(&now, NULL)) {
		gru_status_strerror(status, GRU_FAILURE, errno);

		return -1;
	}

	return ((pn_timestamp_t) now.tv_sec) * 1000 + (now.tv_usec / 1000);
}

static vmsl_stat_t proton_do_send(pn_messenger_t *messenger,
	pn_message_t *message,
	gru_status_t *status) {
	mpt_trace("Putting message");
	pn_messenger_put(messenger, message);
	if (failed(messenger)) {
		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);
		gru_status_set(status, GRU_FAILURE, protonErrorText);

		return VMSL_ERROR;
	}

	pn_messenger_send(messenger, -1);
	if (failed(messenger)) {
		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);
		gru_status_set(status, GRU_FAILURE, protonErrorText);
		return VMSL_ERROR;
	}

	return VMSL_SUCCESS;
}

vmsl_stat_t
	proton_send(msg_ctxt_t *ctxt, msg_content_data_t *data, gru_status_t *status) {
	vmsl_stat_t ret = {0};

	mpt_trace("Creating message object");
	pn_message_t *message = pn_message();

	// proton_set_message_properties(ctxt, message, status);
	mpt_trace("Setting message address to %s", url);
	pn_message_set_address(message, url);

	pn_data_t *body = pn_message_body(message);
	pn_data_put_string(body, pn_bytes(data->size, data->data));

	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	vmslh_run(proton_ctxt->handlers->before_send, proton_ctxt, message);

	pn_message_set_creation_time(message, proton_now(status));
	ret = proton_do_send(proton_ctxt->messenger, message, status);
	if (vmsl_stat_error(ret)) {
		pn_message_free(message);
		return ret;
	}

	vmslh_run(proton_ctxt->handlers->after_send, proton_ctxt, message);

	pn_message_free(message);
	return VMSL_SUCCESS;
}

vmsl_stat_t proton_subscribe(msg_ctxt_t *ctxt, vmsl_mtopic_spec_t *mtopic, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	logger(INFO, "Subscribing to endpoint address at %s", url);
	pn_messenger_subscribe(proton_ctxt->messenger, url);
	if (failed(proton_ctxt->messenger)) {
		pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);

		const char *protonErrorText = pn_error_text(error);
		gru_status_set(status, GRU_FAILURE, protonErrorText);

		return VMSL_ERROR;
	}

	pn_messenger_set_incoming_window(proton_ctxt->messenger, window);
	pn_messenger_set_blocking(proton_ctxt->messenger, false);
	return VMSL_SUCCESS;
}

static vmsl_stat_t proton_receive_local(pn_messenger_t *messenger, gru_status_t *status) {
	const int limit = 1024;

	// mpt_trace("Receiving at most %i messages", limit);
	int ret = pn_messenger_recv(messenger, limit);
	if (ret != 0) {
		if (failed(messenger)) {
			pn_error_t *error = pn_messenger_error(messenger);

			const char *protonErrorText = pn_error_text(error);
			mpt_trace("Error receiving messages: %s", protonErrorText);

			gru_status_set(status, GRU_FAILURE, protonErrorText);
			return VMSL_ERROR;
		} else {
			// mpt_trace("No messages to receive");
			return VMSL_SUCCESS | VMSL_NO_DATA;
		}
	}

	return VMSL_SUCCESS;
}

static int proton_do_receive(pn_messenger_t *messenger,
	pn_message_t *message,
	msg_content_data_t *content) {

	pn_messenger_get(messenger, message);
	if (failed(messenger)) {
		logger_t logger = gru_logger_get();

		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);
		logger(ERROR, protonErrorText);

		return 1;
	}

	pn_data_t *body = pn_message_body(message);

	content->size = content->capacity;
	pn_data_format(body, content->data, &content->size);
	if (failed(messenger)) {
		logger_t logger = gru_logger_get();
		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);
		logger(ERROR, protonErrorText);

		return 1;
	}

	mpt_trace("Received data (%d bytes): %s", content->size, content->data);
	return 0;
}

vmsl_stat_t
	proton_receive(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);
	static int nmsgs = 0; // Number of messages in the local queue
	static int cur = 0; // Current message being processed in the batch
	static int last_ack = 0; // Last acknowledged/settled message in the batch

	// First check if there are messages in the local buffer
	nmsgs = pn_messenger_incoming(proton_ctxt->messenger);
	if (nmsgs == 0) {
		// If not, try to receive from the remote peer
		vmsl_stat_t local_ret = proton_receive_local(proton_ctxt->messenger, status);
		if (local_ret == VMSL_ERROR) {
			return local_ret;
		} else {
			if (local_ret & VMSL_NO_DATA) {
				return local_ret;
			}
		}

		nmsgs = pn_messenger_incoming(proton_ctxt->messenger);
		cur = 0;
		last_ack = 0;

		if (nmsgs == 0) {
			return VMSL_SUCCESS | VMSL_NO_DATA;
		}
	}

	pn_message_t *message = pn_message();

	vmslh_run(proton_ctxt->handlers->before_receive, proton_ctxt, message);

	int ret = proton_do_receive(proton_ctxt->messenger, message, content);

	if (ret == 0 && (ctxt->msg_opts.statistics & MSG_STAT_LATENCY)) {
		pn_timestamp_t proton_ts = pn_message_get_creation_time(message);

		if (proton_ts > 0) {
			mpt_trace("Creation timestamp collected");
			content->created = gru_time_from_milli(proton_ts);
		} else {
			sleep(1);
			logger(DEBUG, "Unable to collect creation timestamp: %" PRId64 " is invalid",
				proton_ts);
			gru_status_set(status, GRU_FAILURE, "A timestamp was not set for a message");

			pn_message_free(message);
			return VMSL_ERROR | VMSL_NO_TIMESTAMP;
		}
	} else if (ret != 0) {
		gru_status_set(status, GRU_FAILURE, "Error receiving a message");
		goto err_exit;
	}
	cur++;

	vmslh_run(proton_ctxt->handlers->after_receive, proton_ctxt, message);

	// Settles the messages after every 'window' count
	if ((last_ack + window) == cur) {
		mpt_trace("Acknowledging message: %i current (%i messages / %i last ack)",
			cur,
			nmsgs,
			last_ack);


		vmslh_run(proton_ctxt->handlers->finalize_receive, proton_ctxt, NULL);

		cur = 0;
		last_ack = 0;
	} else {
		// Otherwise, if at the end of the local buffer, settle the remaining
		if (nmsgs == 1) {
			mpt_trace(
				"Acknowledging remaining messages on the batch: %i current (%i messages / %i last ack)",
				cur,
				nmsgs,
				last_ack);

			vmslh_run(proton_ctxt->handlers->finalize_receive, proton_ctxt, NULL);

			cur = 0;
			last_ack = 0;
		}
	}

	pn_message_free(message);
	return VMSL_SUCCESS;

err_exit:
	pn_message_free(message);
	return VMSL_ERROR;
}

bool proton_vmsl_assign(vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Initializing AMQP protocol");

	vmsl->init = proton_init;
	vmsl->start = proton_start;
	vmsl->receive = proton_receive;
	vmsl->subscribe = proton_subscribe;
	vmsl->send = proton_send;
	vmsl->stop = proton_stop;
	vmsl->destroy = proton_destroy;

	return true;
}
