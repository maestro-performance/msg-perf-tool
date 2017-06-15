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

static void proton_set_parameter_by_name(vmslh_handlers_t *handlers, gru_keypair_t *kp, gru_status_t *status) {
	logger_t logger = gru_logger_get();

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

			logger(INFO, "Parsed property: %s -> %s", property->key, property->pair->variant.string);
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
}

void proton_set_user_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status) {
	gru_uri_t uri = opt.uri;

	if (!uri.query) {
		return;
	}

	gru_node_t *node = uri.query->root;

	while (node) {
		gru_keypair_t *kp = (gru_keypair_t *) node->data;
		proton_set_parameter_by_name(handlers, kp, status);

		node = node->next;
	}
}


void proton_set_default_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status) {
	vmslh_add(handlers->before_send, proton_set_default_message_properties, NULL, status);
}

void proton_set_properties(void *ctxt, void *msg, void *payload) {
	gru_list_t *properties = (gru_list_t *) payload;
	pn_message_t *message = (pn_message_t*) msg;

	pn_data_t *msg_properties = pn_message_properties(message);

	pn_data_put_map(msg_properties);
	pn_data_enter(msg_properties);

	gru_node_t *node = properties->root;

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

	pn_message_set_content_type(message, (char *) payload);
}

void proton_set_ttl(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;
	gru_variant_t *variant = (gru_variant_t *) payload;

	logger_t logger = gru_logger_get();
	logger(INFO, "Setting the TTL to %d", variant->variant.inumber);

	pn_message_set_ttl(message, variant->variant.inumber);
}

void proton_set_default_message_properties(void *ctxt, void *msg, void *payload) {
	pn_message_t *message = (pn_message_t *) msg;

	pn_message_set_durable(message, false);
	pn_message_set_ttl(message, 50000);

	pn_message_set_content_type(message, "text/plain");
}