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
#include "paho-wrapper.h"
#include "paho-context.h"
#include "vmsl.h"

static inline paho_ctxt_t *paho_ctxt_cast(msg_ctxt_t *ctxt) {
	return (paho_ctxt_t *) ctxt->api_context;
}

msg_ctxt_t *paho_init(stat_io_t *stat_io, msg_opt_t opt, void *data, gru_status_t *status) {
    logger_t logger = gru_logger_get();

    logger(DEBUG, "Initializing Paho wrapper");

    msg_ctxt_t *msg_ctxt = msg_ctxt_init(stat_io, status);
    if (!msg_ctxt) {
            logger(FATAL, "Unable to initialize the messaging context");

            exit(1);
    }

    paho_ctxt_t *paho_ctxt = paho_context_init();

    if (!paho_ctxt) {
            logger(FATAL, "Unable to initialize the proton context");

            exit(1);
    }

    const options_t *options = get_options_object();
    paho_ctxt->uri = gru_uri_parse_ex(options->url, GRU_URI_PARSE_STRIP, status);

	if (!gru_uri_set_scheme(&paho_ctxt->uri, "tcp")) {
        logger(FATAL, "Unable to adjust the connection URI");

        exit(1);
    }

    const char *connect_url = gru_uri_format(&paho_ctxt->uri, GRU_URI_FORMAT_NONE,
											 status);

	logger(DEBUG, "Creating a client to %s", connect_url);
	int rc = 0;
	if (opt.direction == MSG_DIRECTION_SENDER) {
		rc = MQTTClient_create(&paho_ctxt->client, connect_url, "msg-perf-tool-sender",
							 MQTTCLIENT_PERSISTENCE_NONE, NULL);
	}
	else {
		rc = MQTTClient_create(&paho_ctxt->client, connect_url, "msg-perf-tool-receiver",
							 MQTTCLIENT_PERSISTENCE_NONE, NULL);
	}


	if (rc != MQTTCLIENT_SUCCESS) {
        logger(FATAL, "Unable to create MQTT client handle: %d", rc);

        exit(-1);
    }

	logger(DEBUG, "Setting connection options");
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

	logger(DEBUG, "Connecting to %s", connect_url);
    rc = MQTTClient_connect(paho_ctxt->client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        logger(FATAL, "Unable to connect: %d", rc);

        exit(-1);
    }

    msg_ctxt->api_context = paho_ctxt;

    return msg_ctxt;
}


void paho_stop(msg_ctxt_t *ctxt) {
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	MQTTClient_destroy(&paho_ctxt->client);
}


void paho_destroy(msg_ctxt_t *ctxt) {
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	MQTTClient_disconnect(paho_ctxt->client, 10000);
}

vmsl_stat_t paho_send(msg_ctxt_t *ctxt, msg_content_loader content_loader, gru_status_t *status) {
	MQTTClient_deliveryToken token;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	msg_content_data_t msg_content;

	content_loader(&msg_content);

	pubmsg.payload = msg_content.data;
	pubmsg.payloadlen = msg_content.size;

	// QoS0, At most once:
	pubmsg.qos = 0;
	pubmsg.retained = false;

	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	mpt_trace("Sending message to %s", paho_ctxt->uri.path);

	int rc = MQTTClient_publishMessage(paho_ctxt->client, paho_ctxt->uri.path,
		&pubmsg, &token);

	switch (rc) {
		case MQTTCLIENT_SUCCESS: break;
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to publish the message: error %d",
						 rc);

			return VMSL_ERROR;
		}
	}

	rc = MQTTClient_waitForCompletion(paho_ctxt->client, token, TIMEOUT);
	switch (rc) {
		case MQTTCLIENT_SUCCESS: break;
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to synchronize: error %d",
						 rc);

			return VMSL_ERROR;
		}
	}

	mpt_trace("Delivered message %d", token);
	return VMSL_SUCCESS;
}


vmsl_stat_t paho_subscribe(msg_ctxt_t *ctxt, void *data, gru_status_t *status) {
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);

	logger_t logger = gru_logger_get();

	logger(DEBUG, "Subscribing to %s", paho_ctxt->uri.path);

	int rc = MQTTClient_subscribe(paho_ctxt->client, paho_ctxt->uri.path, QOS_AT_MOST_ONCE);

	switch (rc) {
		case MQTTCLIENT_SUCCESS: break;
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to subscribe: error %d", rc);

			return VMSL_ERROR;
		}
	}

	logger(DEBUG, "Subscribed to the topic");
	return VMSL_SUCCESS;
}


vmsl_stat_t paho_receive(msg_ctxt_t *ctxt, msg_content_data_t *content,
						 gru_status_t *status) {
	MQTTClient_message *msg = NULL;
	paho_ctxt_t *paho_ctxt = paho_ctxt_cast(ctxt);
	unsigned long timeout = 10000L;

	int tlen = 0;
	char *topic_name;
	int rc = MQTTClient_receive(paho_ctxt->client, &topic_name, &tlen, &msg,
							 timeout);

	switch (rc) {
		case MQTTCLIENT_SUCCESS:  {
			if (!msg) {
				return VMSL_SUCCESS | VMSL_NO_DATA;
			}

			return VMSL_SUCCESS;
		}
		case MQTTCLIENT_TOPICNAME_TRUNCATED: {
			logger_t logger = gru_logger_get();

			logger(WARNING, "Topic name truncated");
			break;
		}
		default: {
			gru_status_set(status, GRU_FAILURE, "Unable to receive data: error %d", rc);

			return VMSL_ERROR;
		}
	}

	return VMSL_SUCCESS;
}
