/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "proton-wrapper.h"
#include "proton-context.h"


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