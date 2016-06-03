/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "msgctxt.h"

msg_ctxt_t *msg_ctxt_init() {
    msg_ctxt_t *ret = malloc(sizeof(msg_ctxt_t));
    
    logger_t logger = get_logger();
    
    if (ret == NULL) {
        logger(FATAL, "Unable to initialize messaging context object");
        
        exit(1);
    }

    return ret;
}


void msg_ctxt_destroy(msg_ctxt_t **ctxt) {
    free(*ctxt);
    *ctxt = NULL;
}