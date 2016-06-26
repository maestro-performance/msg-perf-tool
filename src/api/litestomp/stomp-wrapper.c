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


static inline stomp_ctxt_t *stomp_ctxt_cast(msg_ctxt_t *ctxt)
{
    return (stomp_ctxt_t *) ctxt->api_context;
}


msg_ctxt_t *stomp_init(void *data) {
    logger_t logger = get_logger();

    logger(DEBUG, "Initializing proton wrapper");

    msg_ctxt_t *msg_ctxt = msg_ctxt_init();
    if (!msg_ctxt) {
        logger(FATAL, "Unable to initialize the messaging context");

        exit(1);
    }

    stomp_ctxt_t *stomp_ctxt = stomp_context_init();

    if (!stomp_ctxt_t) {
        logger(FATAL, "Unable to initialize the stomp context");

        exit(1);
    }

        
    stomp_messenger_t *messenger = stomp_messenger_init(); 
    
    if (!messenger) {
       fprintf(stderr, "Unable to initialize stomp messenger\n");
                
       exit(1);
    }
    
    
    const options_t *options = get_options_object();
    
    /*
     * Sets the endpoint address
     */
    stomp_status_code_t stat = stomp_set_endpoint(messenger, options->url);
    if (stat != STOMP_SUCCESS) {
        fprintf(stderr, "%s\n", messenger->status.message);
              
        exit(1);
    }
    

    stomp_ctxt_t->messenger = messenger;
    msg_ctxt->api_context = stomp_ctxt_t;

    return msg_ctxt;
}


void stomp_stop(msg_ctxt_t *ctxt) {
    // NO-OP
}


void stomp_destroy(msg_ctxt_t *ctxt) {
    stomp_ctxt_t *stomp_ctxt = stomp_ctxt_cast(ctxt);
    
    stomp_messenger_destroy(&stomp_ctxt->messenger);
    msg_ctxt_destroy(&ctxt);
}

void stomp_send(msg_ctxt_t *ctxt, msg_content_loader content_loader) {
    stomp_ctxt_t *stomp_ctxt = stomp_ctxt_cast(ctxt);
    logger_t logger = get_logger();

    logger(TRACE, "Creating message object");
    
     /*
     * Creates a message to be sent
     */
    stomp_message_t *message = stomp_message_create(&stomp_ctxt->messenger->status);
    if (!message) {
        fprintf(stderr, "%s\n", stomp_ctxt->messenger->status.message);

        // TODO: handle error
    }


    msg_content_data_t msg_content;
    
    content_loader(&msg_content);
    
    /*
     * Formats the message
     */
    
    stomp_message_format(message, msg_content.data, msg_content.size);

    stomp_send_header_t send_header;

    send_header.transaction_id = -1;
    
    stomp_status_code_t stat = stomp_exchange_util_ctime(stomp_ctxt->messenger->exchange_properties,
                                     &stomp_ctxt->messenger->status);
    if (stat != STOMP_SUCCESS) {
        fprintf(stderr, stomp_ctxt->messenger->status.message);

        // TODO: handle error
    }

    /*
     * Sends the message
     */
    stat = stomp_send(stomp_ctxt->messenger, &send_header, message);
    if (stat != STOMP_SUCCESS) {
        fprintf(stderr, "%s\n", stomp_ctxt->messenger->status.message);

        // TODO: handle error
    }
}


void stomp_subscribe(msg_ctxt_t *ctxt, void *data) {
    stomp_ctxt_t *stomp_ctxt = stomp_ctxt_cast(ctxt);
    /*
     * Subscribes to the endpoint. Uses a fake ID and receipt just for the sake
     * of the example
     */
    stomp_subscription_header_t sub_header;
    
    
    stomp_status_code_t stat = stomp_subscribe(stomp_ctxt->messenger, &sub_header);
    if (stat != STOMP_SUCCESS) {
        fprintf(stderr, "%s\n", stomp_ctxt->messenger->status.message);
        
        // TODO: handle error
    }
}


void stomp_receive(msg_ctxt_t *ctxt, msg_content_data_t *content) {

}