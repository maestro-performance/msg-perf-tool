/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_sender.h"


static const msg_content_data_t *content_loader() {
    msg_content_data_t *ret = malloc(sizeof(msg_content_data_t));
    
    ret->size = 10;
    ret->data = "0123456789";
    
    return ret;
}

void sender_start(const vmsl_t *vmsl, const options_t *options) {
	msg_ctxt_t *msg_ctxt = vmsl->init(NULL);
	
	vmsl->send(msg_ctxt, content_loader);
}