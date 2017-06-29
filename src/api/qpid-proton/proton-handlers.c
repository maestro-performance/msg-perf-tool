/**
 Copyright 2017 Otavio Rodolfo Piske

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
#include <common/gru_variant.h>
#include "proton-handlers.h"

gru_list_t *properties = NULL;

void proton_param_cleanup() {
	if (!properties) {
		return;
	}

	gru_list_clean(properties, gru_keypair_destroy_list_item);
	gru_list_destroy(&properties);
}

static void proton_set_parameter_by_name(vmslh_handlers_t *handlers, gru_keypair_t *kp, msg_opt_t opt, gru_status_t *status) {
	if (gru_keypair_key_equals(kp, "content-type")) {
		vmslh_add(handlers->before_send, proton_set_content_type, kp->pair->variant.string, status);
	}

	if (gru_keypair_key_equals(kp, "application-properties")) {
		if (!properties) {
			properties = gru_list_new(status);
		}

		gru_list_t *tmp = gru_split(kp->pair->variant.string, ',', status);

		gru_node_t *node = tmp->root;
		while (node) {
			gru_keypair_t *property = gru_keypair_parse((const char *) node->data, status);

			mpt_trace("Parsed property: %s -> %s", property->key, property->pair->variant.string);
			gru_list_append(properties, property);
			node = node->next;
		}

		vmslh_add(handlers->before_send, proton_set_properties, properties, status);

		gru_split_clean(tmp);
		gru_list_destroy(&tmp);
	}

	if (gru_keypair_key_equals(kp, "ttl")) {
		vmslh_add(handlers->before_send, proton_set_ttl, kp->pair, status);
	}

	if (gru_keypair_key_equals(kp, "durable")) {
		vmslh_add(handlers->before_send, proton_set_durable, kp->pair, status);
	}

	if (gru_keypair_key_equals(kp, "priority")) {
		vmslh_add(handlers->before_send, proton_set_priority, kp->pair, status);
	}

	if (gru_keypair_key_equals(kp, "qos-mode")) {
		if (opt.direction == MSG_DIRECTION_SENDER) {
			vmslh_add(handlers->before_connect, proton_set_qos_mode_send, kp->pair, status);
		} else {
			vmslh_add(handlers->before_connect, proton_set_qos_mode_recv, kp->pair, status);
		}
	}
}

void proton_set_user_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status) {
	gru_uri_t uri = opt.uri;

	if (!uri.query) {
		return;
	}

	gru_node_t *node = uri.query->root;

	while (node) {
		gru_keypair_t *kp = (gru_keypair_t *) node->data;
		proton_set_parameter_by_name(handlers, kp, opt, status);

		node = node->next;
	}
}


void proton_set_default_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status) {
	if (opt.direction == MSG_DIRECTION_SENDER) {
		vmslh_add(handlers->before_connect, proton_set_qos_mode_send, NULL, status);
	}
	else {
		vmslh_add(handlers->before_connect, proton_set_qos_mode_recv, NULL, status);
	}

	vmslh_add(handlers->before_send, proton_set_ttl, NULL, status);
	vmslh_add(handlers->before_send, proton_set_durable, NULL, status);
	vmslh_add(handlers->before_send, proton_set_content_type, NULL, status);
}

void proton_set_properties(void *ctxt, void *msg, void *payload) {
	gru_list_t *pl_properties = (gru_list_t *) payload;
	pn_message_t *message = (pn_message_t*) msg;

	pn_data_t *msg_properties = pn_message_properties(message);

	pn_data_put_map(msg_properties);
	pn_data_enter(msg_properties);

	gru_node_t *node = pl_properties->root;

	while (node) {
		gru_keypair_t *property = (gru_keypair_t *) node->data;

		pn_data_put_string(msg_properties, pn_bytes(strlen(property->key), property->key));
		pn_data_put_string(msg_properties, pn_bytes(strlen(property->pair->variant.string),
													property->pair->variant.string));

		node = node->next;
	}

	pn_data_exit(msg_properties);
}

// Probably only for debugging
void proton_log_body_type(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;
	pn_data_t *body = pn_message_body(message);

	if (!body) {
		return;
	}

	logger_t logger = gru_logger_get();

	switch (pn_data_type(body)) {
	case PN_STRING:
		logger(INFO, "Message body type is string");
		break;
	case PN_INT:
		logger(INFO, "Message body type is int");
		break;
	case PN_MAP:
		logger(INFO, "Message body type is map");
		break;
	case PN_LIST:
		logger(INFO, "Message body type is list");
		break;
	case PN_ARRAY:
		logger(INFO, "Message body type is array");
		break;
	case PN_NULL:
		logger(INFO, "Message body type is null");
		break;
	case PN_BOOL:
		logger(INFO, "Message body type is boolean");
		break;
	case PN_UBYTE:
		logger(INFO, "Message body type is unsigned byte");
		break;
	case PN_USHORT:
		logger(INFO, "Message body type is unsigned short");
		break;
	case PN_UINT:
		logger(INFO, "Message body type is unsigned int");
		break;
	case PN_ULONG:
		logger(INFO, "Message body type is unsigned long");
		break;
	case PN_LONG:
		logger(INFO, "Message body type is long");
		break;
	case PN_FLOAT:
		logger(INFO, "Message body type is float");
		break;
	case PN_BINARY:
		logger(INFO, "Message body type is binary");
		break;
	case PN_SYMBOL:
		logger(INFO, "Message body type is symbol");
		break;
	default:
		logger(INFO, "Message body type is undefined");
		break;
	}
}

void proton_set_content_type(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;

	if (!payload) {
		pn_message_set_content_type(message, "text/plain");
	} else {
		pn_message_set_content_type(message, (char *) payload);
	}
}

void proton_set_ttl(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;
	gru_variant_t *variant = (gru_variant_t *) payload;

	if (payload == NULL) {
		pn_message_set_ttl(message, (pn_millis_t) 5000);
	}
	else {
		mpt_trace("Setting the TTL to %d", variant->variant.inumber);

		pn_message_set_ttl(message, (pn_millis_t) variant->variant.inumber);
	}
}

void proton_set_durable(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;

	if (!payload) {
		pn_message_set_durable(message, false);
	} else {
		gru_variant_t *variant = (gru_variant_t *) payload;

		mpt_trace("Setting the durable to %s", (variant->variant.flag ? "true" : "false"));

		pn_message_set_durable(message, variant->variant.flag);
	}
}

void proton_set_priority(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;
	gru_variant_t *variant = (gru_variant_t *) payload;

	if (variant->type == GRU_INTEGER) {
		mpt_trace("Setting the priority to %d", variant->variant.inumber);

		pn_message_set_priority(message, (uint8_t) variant->variant.inumber);
	} else {
		uint8_t priority = (uint8_t ) (rand() % 9);

		logger_t logger = gru_logger_get();

		logger(INFO, "Setting the priority to %d", priority);
		pn_message_set_priority(message, priority);
	}
}

void proton_set_qos_mode_send(void *ctxt, void *msg, void *payload) {
	gru_variant_t *variant = (gru_variant_t *) payload;
	proton_ctxt_t *proton_ctxt = (proton_ctxt_t *) ctxt;
	logger_t logger = gru_logger_get();

	if (payload == NULL || gru_variant_equals_str(variant, "at-most-once")) {
		/**
		 * From the documentation:
		 *
		 * Sender presettles (aka at-most-once): "... in this configuration the sender
		 * settles (i.e. forgets about) the delivery before it even reaches the
		 * receiver,
		 * and if anything should happen to the delivery in-flight, there is no way to
		 * recover, hence the "at most once" semantics ..."
		 */
		mpt_trace("Using At most once");
		pn_messenger_set_snd_settle_mode(proton_ctxt->messenger, PN_SND_SETTLED);

		return;
	} else {
		logger(WARNING, "Using an unsupported QOS mode");

		pn_messenger_set_outgoing_window(proton_ctxt->messenger, 1);
		pn_messenger_set_snd_settle_mode(proton_ctxt->messenger, PN_SND_UNSETTLED);

		/**
		 * If we have something else, then we need to ensure we settle the delivery
		 * after it has been sent. Because this is done, here, before the connection
		 * occurs, there should be no noticeable impact on the delivery performance
		 */
		gru_status_t status = gru_status_new();
		vmslh_add(proton_ctxt->handlers->after_send, proton_commit, NULL, &status);
		if (gru_status_error(&status)) {
			logger(ERROR, "Unable to add the commit handler for the given QOS mode");
		}
	}
}


void proton_set_qos_mode_recv(void *ctxt, void *msg, void *payload) {
	gru_variant_t *variant = (gru_variant_t *) payload;
	proton_ctxt_t *proton_ctxt = (proton_ctxt_t *) ctxt;

	logger_t logger = gru_logger_get();

	if (payload == NULL || gru_variant_equals_str(variant, "at-most-once")) {
		gru_status_t status = gru_status_new();

		vmslh_add(proton_ctxt->handlers->finalize_receive, proton_accept, NULL, &status);
		if (gru_status_error(&status)) {
			logger(ERROR, "Unable to add the commit handler for the given QOS mode");
		}
	}

	if (gru_variant_equals_str(variant, "at-least-once")) {
		/**
		 * From the documentation: "... In this configuration the receiver settles the
		 * delivery first, and the sender settles once it sees the receiver has settled.
		 * Should anything happen to the delivery in-flight, the sender can resend,
		 * however the receiver may have already forgotten the delivery and so it could
		 * interpret the resend as a new delivery, hence the "at least once" semantics ..."
		 */
		logger(INFO, "Setting QOS to At least once");
		pn_messenger_set_rcv_settle_mode(proton_ctxt->messenger, PN_RCV_FIRST);

		return;
	}

	if (gru_variant_equals_str(variant, "exactly-once")) {
		/**
		 * From the documentation: "... In this configuration the receiver settles only
		 * once it has seen that the sender has settled. This provides the sender the
		 * option to retransmit, and the receiver has the option to recognize (and
		 * discard) duplicates, allowing for exactly once semantics ..."
		 */
		logger(INFO, "Setting QOS to exactly once");
		pn_messenger_set_rcv_settle_mode(proton_ctxt->messenger, PN_RCV_SECOND);

		return;
	}
}

gru_attr_unused static void proton_check_status(pn_messenger_t *messenger,
												pn_tracker_t tracker) {
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


static void proton_do_commit(pn_messenger_t *messenger) {
	pn_tracker_t tracker = pn_messenger_outgoing_tracker(messenger);

	mpt_trace("Committing the message delivery");

#if defined(MPT_DEBUG) && MPT_DEBUG >= 1
	// proton_check_status(messenger, tracker);
#endif
	pn_messenger_settle(messenger, tracker, 0);

#if defined(MPT_DEBUG) && MPT_DEBUG >= 1
	// proton_check_status(messenger, tracker);
#endif
}

void proton_commit(void *ctxt, void *msg, void *payload) {
	proton_ctxt_t *proton_ctxt = (proton_ctxt_t *) ctxt;

	proton_do_commit(proton_ctxt->messenger);
}


static void proton_do_accept(pn_messenger_t *messenger) {
	pn_tracker_t tracker = pn_messenger_incoming_tracker(messenger);

	mpt_trace("Accepting the message delivery");

#if defined(MPT_DEBUG) && MPT_DEBUG >= 1
	proton_check_status(messenger, tracker);
#endif
	pn_messenger_accept(messenger, tracker, PN_CUMULATIVE);
	pn_messenger_settle(messenger, tracker, PN_CUMULATIVE);

#if defined(MPT_DEBUG) && MPT_DEBUG >= 1
	proton_check_status(messenger, tracker);
#endif
}

void proton_accept(void *ctxt, void *msg, void *payload) {
	proton_ctxt_t *proton_ctxt = (proton_ctxt_t *) ctxt;

	proton_do_accept(proton_ctxt->messenger);
}