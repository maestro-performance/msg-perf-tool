/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   proton-wrapper.h
 * Author: otavio
 *
 * Created on May 30, 2016, 9:30 PM
 */

#ifndef PROTON_WRAPPER_H
#define PROTON_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "msgctxt.h"
    
#include <proton/messenger.h>
   
msg_ctxt_t *proton_init(void *data);
        
        
/*
 typedef msg_ctxt_t *(*msg_init)(void *data); 
typedef void(*msg_send)(msg_ctxt_t *ctxt, void *data);
typedef void(*msg_commit)(msg_ctxt_t *ctxt, void *data);
 */


#ifdef __cplusplus
}
#endif

#endif /* PROTON_WRAPPER_H */

