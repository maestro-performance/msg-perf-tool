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
#include "proton-wrapper.h"
#include "proton-context.h"
#include "vmsl.h"

static inline bool failed(pn_messenger_t *messenger) {
	if (pn_messenger_errno(messenger)) {
		return true;
	}

	return false;
}

static inline proton_ctxt_t *proton_ctxt_cast(msg_ctxt_t *ctxt) {
	return (proton_ctxt_t *) ctxt->api_context;
}

msg_ctxt_t *proton_init(stat_io_t *stat_io, void *data, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Initializing proton wrapper");

	msg_ctxt_t *msg_ctxt = msg_ctxt_init(stat_io, status);
	if (!msg_ctxt) {
		return NULL;
	}

	proton_ctxt_t *proton_ctxt = proton_context_init();

	if (!proton_ctxt) {
		logger(FATAL, "Unable to initialize the proton context");

		exit(1);
	}

	pn_messenger_t *messenger = pn_messenger(NULL);

	logger(DEBUG, "Initializing the proton messenger");
	int err = pn_messenger_start(messenger);
	if (err) {
		logger(FATAL, "Unable to start the proton messenger");

		exit(1);
	}

	proton_ctxt->messenger = messenger;
	msg_ctxt->api_context = proton_ctxt;

	return msg_ctxt;
}

void proton_stop(msg_ctxt_t *ctxt, gru_status_t *status) {
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	pn_messenger_stop(proton_ctxt->messenger);
}

void proton_destroy(msg_ctxt_t *ctxt, gru_status_t *status) {
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	pn_messenger_free(proton_ctxt->messenger);
	proton_context_destroy(&proton_ctxt);

	msg_ctxt_destroy(&ctxt);
}

static void proton_check_status(pn_messenger_t *messenger, pn_tracker_t tracker) {
	logger_t logger = gru_logger_get();

	pn_status_t status = pn_messenger_status(messenger, tracker);

	logger(TRACE, "Checking message status");
	switch (status) {
		case PN_STATUS_UNKNOWN: {
			logger(TRACE, "Message status unknown");
			break;
		}
		case PN_STATUS_PENDING: {
			logger(TRACE, "Message status pending");
			break;
		}
		case PN_STATUS_ACCEPTED: {
			logger(TRACE, "Message status accepted");
			break;
		}
		case PN_STATUS_REJECTED: {
			logger(TRACE, "Message status rejected");
			break;
		}
		case PN_STATUS_RELEASED: {
			logger(TRACE, "Message status released");
			break;
		}
		case PN_STATUS_MODIFIED: {
			logger(TRACE, "Message status modified");
			break;
		}
		case PN_STATUS_ABORTED: {
			logger(TRACE, "Message status aborted");
			break;
		}
		case PN_STATUS_SETTLED: {
			logger(TRACE, "Message status settled");
			break;
		}
		default: {
			logger(TRACE, "Message status invalid");
			break;
		}
	}
}

static void proton_commit(pn_messenger_t *messenger, gru_status_t *status) {
	pn_tracker_t tracker = pn_messenger_outgoing_tracker(messenger);

	logger_t logger = gru_logger_get();

	logger(TRACE, "Committing the message delivery");

	proton_check_status(messenger, tracker);
	pn_messenger_settle(messenger, tracker, 0);
	proton_check_status(messenger, tracker);
}

static pn_timestamp_t proton_now() {
	struct timeval now;

	if (gettimeofday(&now, NULL)) {
		// TODO: error handling
	}
	/*
	 tv_sec
	 *
	 */

	return ((pn_timestamp_t) now.tv_sec) * 1000 + (now.tv_usec / 1000);
}

static void proton_set_message_properties(pn_message_t *message) {
	logger_t logger = gru_logger_get();
	const options_t *options = get_options_object();

	logger(DEBUG, "Setting message address to %s", options->url);
	pn_message_set_address(message, options->url);
	pn_message_set_durable(message, false);
	pn_message_set_ttl(message, 50000);

	pn_message_set_creation_time(message, proton_now());

	pn_message_set_first_acquirer(message, true);
}

static void proton_set_message_data(
	pn_message_t *message, msg_content_loader content_loader) {
	logger_t logger = gru_logger_get();

	logger(TRACE, "Formatting message body");

	pn_data_t *body = pn_message_body(message);
	msg_content_data_t msg_content;

	content_loader(&msg_content);

	pn_data_put_string(body, pn_bytes(msg_content.capacity, msg_content.data));
}

static vmsl_stat_t proton_do_send(pn_messenger_t *messenger, pn_message_t *message,
						   gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Putting message");
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

vmsl_stat_t proton_send(msg_ctxt_t *ctxt, msg_content_loader content_loader, gru_status_t *status) {
	vmsl_stat_t ret = {0};

	logger_t logger = gru_logger_get();

	logger(TRACE, "Creating message object");
	pn_message_t *message = pn_message();

	proton_set_message_properties(message);
	proton_set_message_data(message, content_loader);

	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	ret = proton_do_send(proton_ctxt->messenger, message, status);
	if (vmls_stat_error(ret)) {
		return ret;
	}

	proton_commit(proton_ctxt->messenger, status);
	pn_message_free(message);
	return VMSL_SUCCESS;
}

static void proton_accept(pn_messenger_t *messenger) {
	pn_tracker_t tracker = pn_messenger_incoming_tracker(messenger);

	logger_t logger = gru_logger_get();

	logger(TRACE, "Accepting the message delivery");

	proton_check_status(messenger, tracker);
	pn_messenger_accept(messenger, tracker, 0);
	proton_check_status(messenger, tracker);
}

static void proton_set_incoming_messenger_properties(pn_messenger_t *messenger) {

	/*
	 * By setting the incoming window to 1 it, basically, behaves as if
	 * it was working in an auto-accept mode
	 */
	pn_messenger_set_incoming_window(messenger, 1);
	pn_messenger_set_blocking(messenger, true);
}

vmsl_stat_t proton_subscribe(msg_ctxt_t *ctxt, void *data, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	const options_t *options = get_options_object();
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	logger(INFO, "Subscribing to endpoint address at %s", options->url);
	pn_messenger_subscribe(proton_ctxt->messenger, options->url);
	if (failed(proton_ctxt->messenger)) {
		pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);

		const char *protonErrorText = pn_error_text(error);
		gru_status_set(status, GRU_FAILURE, protonErrorText);

		return VMSL_ERROR;
	}

	proton_set_incoming_messenger_properties(proton_ctxt->messenger);
	return VMSL_SUCCESS;
}

static int proton_receive_local(pn_messenger_t *gru_restrict messenger,
								gru_status_t * gru_restrict status)
{
	logger_t logger = gru_logger_get();

	if (!pn_messenger_is_blocking(messenger)) {
		logger(WARNING, "The messenger is not in blocking mode");
	}

	int limit = -1;
	logger(TRACE, "Receiving at most %i messages", limit);
	pn_messenger_recv(messenger, limit);
	if (failed(messenger)) {
		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);

		gru_status_set(status, GRU_FAILURE, protonErrorText);

		return -1;
	}

	int incoming = pn_messenger_incoming(messenger);
	if (incoming == 0) {
		logger(DEBUG, "There are 0 incoming messages");
		return 0;
	}

	logger(TRACE, "Getting %i messages from proton buffer", incoming);
	return incoming;
}

static int proton_do_receive(
	pn_messenger_t *messenger, pn_message_t *message, msg_content_data_t *content) {
	logger_t logger = gru_logger_get();

	pn_messenger_get(messenger, message);
	if (failed(messenger)) {
		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);
		logger(ERROR, protonErrorText);

		return 1;
	}

	pn_data_t *body = pn_message_body(message);

	content->size = content->capacity;
	pn_data_format(body, content->data, &content->size);
	if (failed(messenger)) {
		pn_error_t *error = pn_messenger_error(messenger);

		const char *protonErrorText = pn_error_text(error);
		logger(ERROR, protonErrorText);

		return 1;
	}

	logger(DEBUG, "Received data (%d bytes): %s", content->size, content->data);
	return 0;
}

static mpt_timestamp_t proton_timestamp_to_mpt_timestamp_t(pn_timestamp_t timestamp) {
	mpt_timestamp_t ret = {0};

	double ts = ((double) timestamp / 1000);
	double integral;

	ret.tv_usec = modf(ts, &integral) * 1000000;
	ret.tv_sec = integral;

	logger_t logger = gru_logger_get();

	logger(TRACE, "Returning: %lu / %lu / %f", ret.tv_sec, ret.tv_usec, integral);

	return ret;
}

vmsl_stat_t proton_receive(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status) {
	proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

	int count = proton_receive_local(proton_ctxt->messenger, status);

	if (count <= 0) {
		// No messages received is ok ...
		if (count == 0) {
			return (VMSL_SUCCESS | VMSL_NO_DATA);
		}

		return VMSL_ERROR;
	}

	pn_message_t *message = pn_message();
	int ret = proton_do_receive(proton_ctxt->messenger, message, content);

	if (ret == 0) {
		pn_timestamp_t proton_ts = pn_message_get_creation_time(message);

		if (proton_ts > 0) {
			mpt_timestamp_t created = proton_timestamp_to_mpt_timestamp_t(proton_ts);

			mpt_timestamp_t now = proton_timestamp_to_mpt_timestamp_t(proton_now());

			statistics_latency(ctxt->stat_io, created, now);
			content->count++;
		} else {
			content->errors++;
		}

		proton_accept(proton_ctxt->messenger);
	}

	pn_message_free(message);
	return VMSL_SUCCESS;
}