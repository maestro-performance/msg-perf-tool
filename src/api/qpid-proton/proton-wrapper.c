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
    proton_ctxt_t *proton_ctxt = (proton_ctxt_t *) ctxt->api_context;
    pn_messenger_put(proton_ctxt->messenger, message);
    
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
    
    pn_messenger_send(proton_ctxt->messenger, 1);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);
        
        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
}