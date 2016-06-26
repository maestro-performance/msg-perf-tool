/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   stomp-context.h
 * Author: otavio
 *
 * Created on June 26, 2016, 10:34 AM
 */

#ifndef STOMP_CONTEXT_H
#define STOMP_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <litestomp/stomp_messenger.h>
    
#include "contrib/logger.h"

typedef struct stomp_ctxt_t_ {
    stomp_messenger_t *messenger;
} stomp_ctxt_t;

stomp_ctxt_t *stomp_context_init();
void stomp_context_destroy(stomp_ctxt_t **ctxt);



#ifdef __cplusplus
}
#endif

#endif /* STOMP_CONTEXT_H */

