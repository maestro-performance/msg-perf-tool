/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   msgctx.h
 * Author: opiske
 *
 * Created on May 31, 2016, 9:36 AM
 */

#ifndef MSGCTX_H
#define MSGCTX_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdlib.h>
    
#include "contrib/messenger.h"

typedef struct msg_ctxt_t_ {
    void *payload;
} msg_ctxt_t;

msg_ctxt_t *msg_ctxt_init();


#ifdef __cplusplus
}
#endif

#endif /* MSGCTX_H */

