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
#include <MQTTClient.h>
#include "paho-wrapper.h"

const static int TS_SIZE = 18;

static inline paho_ctxt_t *paho_ctxt_cast(msg_ctxt_t *ctxt) {
	return (paho_ctxt_t *) ctxt->api_context;
}

msg_ctxt_t *paho_init(msg_opt_t opt, vmslh_handlers_t *handlers, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(GRU_DEBUG, "Initializing Paho wrapper");

	msg_ctxt_t *msg_ctxt = msg_ctxt_init(status);
	if (!msg_ctxt) {
		logger(GRU_FATAL, "Unable to initialize the messaging context");

		return NULL;
	}

	paho_ctxt_t *paho_ctxt = paho_context_init(handlers, status);

	if (!paho_ctxt) {
		logger(GRU_FATAL, "Unable to initialize the paho context");

		goto err_exit;
	}

	paho_ctxt->uri = gru_uri_clone(opt.uri, status);

	if (!gru_uri_set_scheme(&paho_ctxt->uri, "tcp")) {
		logger(GRU_FATAL, "Unable to adjust the connection URI");

		goto err_exit;
	}

	paho_set_default_parameters(handlers, opt, status);
	paho_set_user_parameters(handlers, opt, status);


	msg_ctxt->api_context = paho_ctxt;
	msg_ctxt->msg_opts = opt;

	return msg_ctxt;

	err_exit:
	msg_ctxt_destroy(&msg_ctxt);
	return NULL;
}

vmsl_stat_t paho_start(msg_ctxt_t *ctxt, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	const char *connect_url = gru_uri_format(&paho_ctxt->uri, GRU_URI_FORMAT_PORT, status);

	logger(GRU_DEBUG, "Creating a client to %s with path %s ", connect_url,
		   paho_ctxt->uri.path);

	int rc = MQTTClient_create(&paho_ctxt->client,
						   connect_url, ctxt->msg_opts.conn_info.id,
						   MQTTCLIENT_PERSISTENCE_NONE,
						   NULL);

	gru_dealloc_const_string(&connect_url);
	if (rc != MQTTCLIENT_SUCCESS) {
		gru_status_set(status, GRU_FAILURE, "Unable to create MQTT client handle: %d", rc);

		return VMSL_ERROR;
	}

	logger(GRU_DEBUG, "Setting connection options");
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.MQTTVersion = MQTTVERSION_3_1_1;

	vmslh_run(paho_ctxt->handlers->before_connect, paho_ctxt, &conn_opts);

	rc = MQTTClient_connect(paho_ctxt->client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS) {
		gru_status_set(status, GRU_FAILURE, "Unable to connect: %d", rc);

		return VMSL_ERROR;
	}

	vmslh_run(paho_ctxt->handlers->after_connect, paho_ctxt, NULL);


	return VMSL_SUCCESS;
}

void paho_stop(msg_ctxt_t *ctxt, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	int rc = MQTTClient_disconnect(paho_ctxt->client, 10000);
	switch (rc) {
		case MQTTCLIENT_SUCCESS:
			break;
		case MQTTCLIENT_FAILURE:
			logger(GRU_WARNING, "Unhandled failure while disconnecting from the server");
			break;
		case MQTTCLIENT_DISCONNECTED:
			logger(GRU_WARNING, "Already disconnected from the server");
			break;
		default: {
			logger(GRU_WARNING, "Error disconnecting from the server: %d", rc);
			break;
		}
	}
}

void paho_destroy(msg_ctxt_t *ctxt, gru_status_t *status) {
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	MQTTClient_destroy(&paho_ctxt->client);

	gru_uri_cleanup(&paho_ctxt->uri);
	paho_context_destroy(&paho_ctxt);
	msg_ctxt_destroy(&ctxt);
}

struct paho_perf_pl {
	char *data;
	int size;
};

static struct paho_perf_pl paho_serialize_latency_info(msg_content_data_t *msg_content) {
	struct paho_perf_pl ret = {0};
	gru_timestamp_t ts = gru_time_now();

	char *formatted_ts = gru_time_write_str(&ts);

	if (unlikely(asprintf(&ret.data, "%18s%s", formatted_ts, (char *) msg_content->data) == -1)) {
		logger_t logger = gru_logger_get();

		logger(GRU_ERROR, "Not enough memory to format the current date/time");
	}
	else {
		ret.size = TS_SIZE + (int) msg_content->size;
		gru_dealloc_string(&formatted_ts);
	}

	return ret;
}

static void paho_serialize_clean(struct paho_perf_pl *pl) {
	free(pl->data);
	pl->size = 0;
}

vmsl_stat_t paho_send(msg_ctxt_t *ctxt, msg_content_data_t *data, gru_status_t *status) {
	MQTTClient_deliveryToken token;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;

	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	int rc = 0;

	if (ctxt->msg_opts.statistics & MSG_STAT_LATENCY) {
		if (unlikely((data->size + TS_SIZE) > INT_MAX)) {
			gru_status_set(status,
				GRU_FAILURE,
				"Data to big to serialize on MQTT protocol: max %d / size: %ld",
				INT_MAX,
				data->size);

			return VMSL_ERROR;
		}

		struct paho_perf_pl pl = paho_serialize_latency_info(data);

		pubmsg.payload = pl.data;
		pubmsg.payloadlen = pl.size;

		logger_t logger = gru_logger_get();
		logger(GRU_DEBUG,
			"Sending message with latency information '%s' to %s",
			data->data,
			ctxt->msg_opts.uri.path);

		vmslh_run(paho_ctxt->handlers->before_send, paho_ctxt, &pubmsg);

		rc = MQTTClient_publishMessage(
			paho_ctxt->client, ctxt->msg_opts.uri.path, &pubmsg, &token);
		paho_serialize_clean(&pl);
	} else {
		pubmsg.payload = data->data;

		if (unlikely(data->size > INT_MAX)) {
			gru_status_set(status,
				GRU_FAILURE,
				"Data to big to serialize on MQTT protocol: max %d / size: %ld",
				INT_MAX,
				data->size);

			return VMSL_ERROR;
		}

		pubmsg.payloadlen = (int) data->size;

		vmslh_run(paho_ctxt->handlers->before_send, paho_ctxt, &pubmsg);

		rc = MQTTClient_publishMessage(
			paho_ctxt->client, ctxt->msg_opts.uri.path, &pubmsg, &token);
	}

	vmslh_run(paho_ctxt->handlers->after_send, paho_ctxt, &pubmsg);

	switch (rc) {
		case MQTTCLIENT_SUCCESS:
			break;
		default: {
			gru_status_set(
				status, GRU_FAILURE, "Unable to publish the message: error %d", rc);

			return VMSL_ERROR;
		}
	}

	rc = MQTTClient_waitForCompletion(paho_ctxt->client, token, TIMEOUT);
	switch (rc) {
		case MQTTCLIENT_SUCCESS:
			break;
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to synchronize: error %d", rc);

			return VMSL_ERROR;
		}
	}

	mpt_trace("Delivered message %d", token);
	return VMSL_SUCCESS;
}

vmsl_stat_t paho_subscribe(msg_ctxt_t *ctxt, vmsl_mtopic_spec_t *mtopic, gru_status_t *status) {
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	logger_t logger = gru_logger_get();


	int rc = 0;
	if (!mtopic) {
		logger(GRU_DEBUG, "Subscribing to %s", (paho_ctxt->uri.path ? paho_ctxt->uri.path : "null"));
		 rc = MQTTClient_subscribe(paho_ctxt->client, paho_ctxt->uri.path, QOS_AT_MOST_ONCE);
	}
	else {
		int qos[mtopic->count];

		for (int i = 0; i < mtopic->count; i++) {
			qos[i] = mtopic->qos;
		}

		rc = MQTTClient_subscribeMany(paho_ctxt->client, mtopic->count, mtopic->topics, qos);
	}

	switch (rc) {
		case MQTTCLIENT_SUCCESS:
			break;
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to subscribe: error %d", rc);

			return VMSL_ERROR;
		}
	}

	logger(GRU_DEBUG, "Subscribed to the topic");
	return VMSL_SUCCESS;
}

vmsl_stat_t
	paho_receive(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status) {
	MQTTClient_message *msg = NULL;
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);
	unsigned long timeout = 10000L;

	int tlen = 0;
	char *topic_name;

	vmslh_run(paho_ctxt->handlers->before_receive, paho_ctxt, NULL);

	int rc = MQTTClient_receive(paho_ctxt->client, &topic_name, &tlen, &msg, timeout);

	switch (rc) {
		case MQTTCLIENT_SUCCESS: {
			if (!msg) {
				// No data received, so send a ping to prevent remote disconnect
				MQTTClient_yield();
				return VMSL_SUCCESS | VMSL_NO_DATA;
			}

			break;
		}
		case MQTTCLIENT_TOPICNAME_TRUNCATED: {
			logger_t logger = gru_logger_get();

			logger(GRU_WARNING, "Topic name truncated");
			break;
		}
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to receive data: error %d", rc);
			return VMSL_ERROR;
		}
	}

	vmslh_run(paho_ctxt->handlers->after_receive, paho_ctxt, msg);

	if (ctxt->msg_opts.statistics & MSG_STAT_LATENCY) {
		char header[18] = {0};
		sscanf(msg->payload, "%17s", header);

		content->created = gru_time_read_str(header);
	}

	if (msg->payloadlen > content->capacity) {
		memcpy(content->data, msg->payload, content->capacity - 1);
	} else {
		memcpy(content->data, msg->payload, msg->payloadlen);
	}
	content->size = msg->payloadlen;

	return VMSL_SUCCESS;
}

vmsl_info_t paho_info() {
	MQTTClient_nameValue *info = MQTTClient_getVersionInfo();

	vmsl_info_t ret = {
		.api_name = info->name,
		.api_version = info->value
	};

	return ret;
}

bool paho_vmsl_assign(vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

	logger(GRU_DEBUG, "Initializing MQTT protocol");

	vmsl->init = paho_init;
	vmsl->start = paho_start;
	vmsl->receive = paho_receive;
	vmsl->subscribe = paho_subscribe;
	vmsl->send = paho_send;
	vmsl->stop = paho_stop;
	vmsl->destroy = paho_destroy;
	vmsl->info = paho_info;

	return true;
}