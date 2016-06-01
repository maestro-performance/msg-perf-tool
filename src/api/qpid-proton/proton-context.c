/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "proton-context.h"

proton_ctxt_t *proton_context_init() {
    proton_ctxt_t *ret = malloc(sizeof(proton_ctxt_t));
    
    if (!ret) {
        logger_t logger = get_logger();
        
        logger(FATAL, "Unable to initialize proton context");
        exit(1);
    }
    
    return ret;
}


void proton_context_destroy(proton_ctxt_t **ctxt) {
    free(*ctxt);
}

