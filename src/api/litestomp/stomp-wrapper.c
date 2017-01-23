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
#include "stomp-wrapper.h"
#include "vmsl.h"

const char *url = NULL;

static inline stomp_ctxt_t *litestomp_ctxt_cast(msg_ctxt_t *ctxt) {
	return (stomp_ctxt_t *) ctxt->api_context;
}

msg_ctxt_t *litestomp_init(stat_io_t *stat_io, msg_opt_t opt, void *data,
						   gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Initializing stomp wrapper");

	msg_ctxt_t *msg_ctxt = msg_ctxt_init(stat_io, status);
	if (!msg_ctxt) {
		return NULL;
	}

	stomp_ctxt_t *stomp_ctxt = litestomp_context_init();

	if (!stomp_ctxt) {
		logger(FATAL, "Unable to initialize the stomp context");

		goto err_exit1;
	}


	const options_t *options = get_options_object();
	url = gru_uri_simple_format(&options->uri, status);
	if (!url) {
		goto err_exit1;
	}

	stomp_messenger_t *messenger = stomp_messenger_init();

	if (!messenger) {
		fprintf(stderr, "Unable to initialize stomp messenger\n");
		goto err_exit1;
	}


	/*
	 * Sets the endpoint address
	 */
	stomp_status_code_t stat = stomp_set_endpoint(messenger, url);
	if (stat != STOMP_SUCCESS) {
		fprintf(stderr, "%s\n", messenger->status.message);

		goto err_exit2;
	}

	/*
	 * Connects to the endpoint
	 */
	stat = stomp_connect(messenger, NULL, 5000);
	if (stat != STOMP_SUCCESS) {
		fprintf(stderr, "%s\n", messenger->status.message);

		goto err_exit2;
	}

	stomp_ctxt->messenger = messenger;
	msg_ctxt->api_context = stomp_ctxt;

	return msg_ctxt;

	err_exit2:
	stomp_messenger_destroy(&messenger);

	err_exit1:
	msg_ctxt_destroy(&msg_ctxt);
	return NULL;
}

void litestomp_stop(msg_ctxt_t *ctxt, gru_status_t *status) {
	// NO-OP
}

void litestomp_destroy(msg_ctxt_t *ctxt, gru_status_t *status) {
	stomp_ctxt_t *stomp_ctxt = litestomp_ctxt_cast(ctxt);

	stomp_messenger_destroy(&stomp_ctxt->messenger);

	litestomp_context_destroy(&stomp_ctxt);
	msg_ctxt_destroy(&ctxt);

	if (url) {
		gru_dealloc_const_string(&url);
	}
}

vmsl_stat_t litestomp_send(msg_ctxt_t *ctxt, msg_content_loader content_loader, gru_status_t *status) {
	stomp_ctxt_t *stomp_ctxt = litestomp_ctxt_cast(ctxt);
	logger_t logger = gru_logger_get();

	logger(TRACE, "Creating message object");

	/*
	* Creates a message to be sent
	*/
	stomp_message_t *message = stomp_message_create(&stomp_ctxt->messenger->status);
	if (!message) {
		logger(ERROR, "Unable to create a stomp message: %s",
			stomp_ctxt->messenger->status.message);

		return VMSL_ERROR;
	}

	msg_content_data_t msg_content;

	content_loader(&msg_content);

	/*
	 * Formats the message
	 */
	stomp_status_code_t stat = stomp_exchange_util_ctime(
		stomp_ctxt->messenger->exchange_properties, &stomp_ctxt->messenger->status);
	if (stat != STOMP_SUCCESS) {
		logger(ERROR, "Unable to set the message creation time: %s",
			stomp_ctxt->messenger->status.message);

		stomp_message_destroy(&message);
		return VMSL_ERROR;
	}

	stomp_message_format(message, msg_content.data, msg_content.size);

	stomp_send_header_t send_header = {0};

	send_header.transaction_id = -1;

	/*
	 * Sends the message
	 */
	stat = stomp_send(stomp_ctxt->messenger, &send_header, message);
	if (stat != STOMP_SUCCESS) {
		logger(ERROR, "Unable to send the message: %s",
			stomp_ctxt->messenger->status.message);

		stomp_message_destroy(&message);
		return VMSL_ERROR;
	}

	stomp_message_destroy(&message);
	return VMSL_SUCCESS;
}

vmsl_stat_t litestomp_subscribe(msg_ctxt_t *ctxt, void *data, gru_status_t *status) {
	stomp_ctxt_t *stomp_ctxt = litestomp_ctxt_cast(ctxt);
	/*
	 * Subscribes to the endpoint. Uses a fake ID and receipt just for the sake
	 * of the example
	 */
	stomp_subscription_header_t sub_header = {0};

	stomp_status_code_t stat = stomp_subscribe(stomp_ctxt->messenger, &sub_header);
	if (stat != STOMP_SUCCESS) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Unable to subscribe to the endpoint: %s",
			stomp_ctxt->messenger->status.message);
		gru_status_set(status, GRU_FAILURE, "Unable to subscribe to the endpoint: %s",
			stomp_ctxt->messenger->status.message);
		return VMSL_ERROR;
	}

	return VMSL_SUCCESS;
}

vmsl_stat_t litestomp_receive(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	stomp_ctxt_t *stomp_ctxt = litestomp_ctxt_cast(ctxt);

	stomp_message_t *message = stomp_message_create(NULL);

	if (!message) {
		logger(ERROR, "Unable to create a stomp message: %s",
			stomp_ctxt->messenger->status.message);

		return VMSL_ERROR;
	}

	stomp_receive_header_t receive_header = {0};
	stomp_status_code_t stat =
		stomp_receive(stomp_ctxt->messenger, &receive_header, message);
	if (stat != STOMP_SUCCESS) {
		logger(ERROR, "Unable to receive messages from the endpoint: %s",
			stomp_ctxt->messenger->status.message);

		stomp_message_destroy(&message);
		return VMSL_ERROR;
	}

	gru_timestamp_t now = gru_time_now();

	const char *ctime = stomp_exchange_get(
		stomp_ctxt->messenger->exchange_properties, STOMP_CREATION_TIME);

	gru_timestamp_t created = gru_time_from_milli_char(ctime);

	content->count++;
	statistics_latency(ctxt->stat_io, created, now);
	stomp_message_destroy(&message);
	return VMSL_ERROR;
}


bool litestomp_vmsl_assign(vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Initializing STOMP protocol");

	vmsl->init = litestomp_init;
	vmsl->receive = litestomp_receive;
	vmsl->subscribe = litestomp_subscribe;
	vmsl->send = litestomp_send;
	vmsl->stop = litestomp_stop;
	vmsl->destroy = litestomp_destroy;

	return true;
}
