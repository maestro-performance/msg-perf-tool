/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_receiver.h"
#include "vmsl.h"

void receiver_start(const vmsl_t *vmsl, const options_t *options)
{
    msg_ctxt_t *msg_ctxt = vmsl->init(NULL);

    
    vmsl->subscribe(msg_ctxt, NULL);
    while (true) {
        vmsl->receive(msg_ctxt);
    }

}