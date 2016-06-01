/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   proton-context.h
 * Author: opiske
 *
 * Created on June 1, 2016, 9:21 AM
 */

#ifndef PROTON_CONTEXT_H
#define PROTON_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "contrib/logger.h"
    
#include <proton/messenger.h>
    
typedef struct proton_ctxt_t_ {
    pn_messenger_t *messenger;
} proton_ctxt_t;

proton_ctxt_t *proton_context_init();
void proton_context_destroy(proton_ctxt_t **ctxt);


#ifdef __cplusplus
}
#endif

#endif /* PROTON_CONTEXT_H */

