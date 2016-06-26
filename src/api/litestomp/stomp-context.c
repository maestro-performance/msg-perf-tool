/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "stomp-context.h"

stomp_ctxt_t *stomp_context_init() {
    stomp_ctxt_t *ret = malloc(sizeof(stomp_ctxt_t));
    
    if (!ret) {
        logger_t logger = get_logger();
        
        logger(FATAL, "Unable to initialize stomp context");
        exit(1);
    }
    
    return ret;
}


void stomp_context_destroy(stomp_ctxt_t **ctxt) {
    free(*ctxt);
    *ctxt = NULL;
}