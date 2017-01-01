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

msg_ctxt_t *paho_init(stat_io_t *stat_io, void *data) {
    /*   
    
    MQTTClient client;
    
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
     */
    logger_t logger = gru_logger_get();

    logger(DEBUG, "Initializing paho wrapper");

    msg_ctxt_t *msg_ctxt = msg_ctxt_init(stat_io);
    if (!msg_ctxt) {
            logger(FATAL, "Unable to initialize the messaging context");

            exit(1);
    }

    paho_ctxt_t *paho_ctxt = paho_context_init();

    if (!paho_ctxt) {
            logger(FATAL, "Unable to initialize the proton context");

            exit(1);
    }

    gru_status_t status = gru_status_new();
    const options_t *options = get_options_object();
    gru_uri_t uri = gru_uri_parse(options->url, &status);
    if (!gru_uri_set_scheme(&uri, "tcp")) {
        logger(FATAL, "Unable to adjust the connection URI");
        
        exit(1);
    }

    gru_uri_set_path(&uri, NULL);
    const char *connect_url = gru_uri_simple_format(&uri, &status);
    

    MQTTClient_create(&paho_ctxt->client, connect_url, "msg-perf-tool",
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    int rc = MQTTClient_connect(paho_ctxt->client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        logger(FATAL, "Unable to connect: %d", rc);
        
        exit(-1);
    }
    
    msg_ctxt->api_context = paho_ctxt;

    return msg_ctxt;
}


void paho_stop(msg_ctxt_t *ctxt) {

}


void paho_destroy(msg_ctxt_t *ctxt) {

}

void paho_send(msg_ctxt_t *ctxt, msg_content_loader content_loader) {

}


void paho_subscribe(msg_ctxt_t *ctxt, void *data) {

}


void paho_receive(msg_ctxt_t *ctxt, msg_content_data_t *content) {

}