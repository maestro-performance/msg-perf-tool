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
#include <MQTTClient.h>
#include <common/gru_variant.h>
#include "paho-handlers.h"

static void paho_set_parameter_by_name(vmslh_handlers_t *handlers, gru_keypair_t *kp, msg_opt_t opt, gru_status_t *status) {
	if (gru_keypair_key_equals(kp, "keep-alive")) {
		vmslh_add(handlers->before_connect, paho_set_keep_alive_interval, kp->pair, status);
	}

	if (gru_keypair_key_equals(kp, "clean-session")) {
		vmslh_add(handlers->before_connect, paho_set_clean_session, kp->pair, status);
	}

	if (gru_keypair_key_equals(kp, "qos-mode")) {
		if (opt.direction == MSG_DIRECTION_SENDER) {
			vmslh_add(handlers->before_send, paho_set_qos_mode, kp->pair, status);
		}
	}

	if (gru_keypair_key_equals(kp, "retained")) {
		vmslh_add(handlers->before_send, paho_set_retained, kp->pair, status);
	}
}

void paho_set_default_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status) {
	vmslh_add(handlers->before_connect, paho_set_keep_alive_interval,
			  PAHO_HANDLERS_DEFAULT_KEEP_ALIVE_PL, status);
	vmslh_add(handlers->before_connect, paho_set_clean_session,
			  PAHO_HANDLERS_DEFAULT_CLEAN_SESSION_PL, status);
	vmslh_add(handlers->before_send, paho_set_qos_mode,
			  PAHO_HANDLERS_DEFAULT_QOS_MODE_PL, status);
	vmslh_add(handlers->before_send, paho_set_retained,
			  PAHO_HANDLERS_DEFAULT_RETAINED_PL, status);
}

void paho_set_user_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status) {
	gru_uri_t uri = opt.uri;

	if (!uri.query) {
		return;
	}

	gru_node_t *node = uri.query->root;

	while (node) {
		gru_keypair_t *kp = (gru_keypair_t *) node->data;
		paho_set_parameter_by_name(handlers, kp, opt, status);

		node = node->next;
	}
}

void paho_set_keep_alive_interval(void *ctxt, void *conn_opts, void *payload) {
	gru_variant_t *variant = (gru_variant_t *) payload;
	MQTTClient_connectOptions *opts = (MQTTClient_connectOptions *) conn_opts;

	if (!payload || variant->type != GRU_INTEGER) {
		opts->keepAliveInterval = 20;
	}
	else {
		opts->keepAliveInterval = (int) variant->variant.inumber;
	}
}

void paho_set_clean_session(void *ctxt, void *conn_opts, void *payload) {
	gru_variant_t *variant = (gru_variant_t *) payload;
	MQTTClient_connectOptions *opts = (MQTTClient_connectOptions *) conn_opts;

	if (!payload || variant->type != GRU_BOOLEAN) {
		opts->cleansession = 1;
	}
	else {
		if (variant->type == GRU_BOOLEAN && variant->variant.flag) {
			opts->cleansession = 1;
		}
		else {
			opts->cleansession = 0;
		}
	}
}

void paho_set_qos_mode(void *ctxt, void *msg, void *payload) {
	MQTTClient_message *pubmsg = (MQTTClient_message *) msg;
	gru_variant_t *variant = (gru_variant_t *) payload;

	if (payload == NULL || gru_variant_equals_str(variant, "at-most-once")) {
		pubmsg->qos = 0;
		return;
	}

	if (gru_variant_equals_str(variant, "at-least-once")) {
		pubmsg->qos = 1;
		return;
	}

	if (gru_variant_equals_str(variant, "exactly-once")) {
		pubmsg->qos = 2;
		return;
	}
}

void paho_set_retained(void *ctxt, void *msg, void *payload) {
	MQTTClient_message *pubmsg = (MQTTClient_message *) msg;
	gru_variant_t *variant = (gru_variant_t *) payload;

	if (payload != NULL && variant->type == GRU_BOOLEAN && variant->variant.flag) {
		pubmsg->retained = true;
	}
	else {
		pubmsg->retained = false;
	}
}