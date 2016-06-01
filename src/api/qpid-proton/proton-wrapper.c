/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "proton-wrapper.h"
#include "proton-context.h"

static bool failed(pn_messenger_t *messenger) {
    if (pn_messenger_errno(messenger)) {
        return true;
    }

    return false;
}


static inline proton_ctxt_t *proton_ctxt_cast(msg_ctxt_t *ctxt) {
    return (proton_ctxt_t *) ctxt->api_context;
}


msg_ctxt_t *proton_init(void *data) {
    logger_t logger = get_logger();
    
    logger(DEBUG, "Initializing proton wrapper");
    
    msg_ctxt_t *msg_ctxt = msg_ctxt_init();
    if (!msg_ctxt) {
        logger(FATAL, "Unable to initialize the messaging context");
        
        exit(1);
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


void proton_send(msg_ctxt_t *ctxt, void *data) {
    logger_t logger = get_logger();
    
    logger(DEBUG, "Creating message object");
    pn_message_t *message = pn_message();
    
    
    const options_t *options = get_options_object();
    logger(DEBUG, "Setting message address to %s", options->url);
    pn_message_set_address(message, options->url);

    logger(DEBUG, "Formatting message body");
    pn_data_t *body = pn_message_body(message);
    pn_data_put_string(body, pn_bytes(5, "1234"));

    logger(DEBUG, "Putting message");
    proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);
    pn_messenger_put(proton_ctxt->messenger, message);
     if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
    
    
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
    
    pn_messenger_send(proton_ctxt->messenger, -1);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
}


void proton_receive(msg_ctxt_t *ctxt) {
    logger_t logger = get_logger();
    proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);
    
    const options_t *options = get_options_object();
    logger(DEBUG, "Subscribing to endpoint address at %s", options->url);
    pn_messenger_subscribe(proton_ctxt->messenger, options->url);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
    
    int count = 1024;
    
    logger(DEBUG, "Receiving %i messages", count);
    pn_messenger_recv(proton_ctxt->messenger, count);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
    
    int incoming = pn_messenger_incoming(proton_ctxt->messenger);
    if (incoming == 0) {
        logger(WARNING, "There are 0 incoming messages");
        return 0;
    }
    
    logger(DEBUG, "Getting %i messages from proton buffer", incoming);
    pn_message_t *message = pn_message();
    pn_messenger_get(proton_ctxt->messenger, message);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
    
    pn_data_t *body = pn_message_body(message);
        
    int buff_size = 1024;
    char *str = malloc(buff_size + 1);
    if (!str) {
        logger(DEBUG, "Unable to allocate memory for the buffer");
        
        exit(1);
    }
    bzero(str, buff_size);
    
    pn_data_format(body, str, &buff_size);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
        
        exit(1);
    }
    
    logger(DEBUG, "Received data (%d bytes): %s", buff_size, str);
    free(str);    
}