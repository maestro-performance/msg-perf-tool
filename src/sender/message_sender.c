/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_sender.h"


static void content_loader(msg_content_data_t *content_data) {
    content_data->size = 10;
    content_data->data = "0123456789";
}

void sender_start(const vmsl_t *vmsl, const options_t *options) {
	msg_ctxt_t *msg_ctxt = vmsl->init(NULL);
	
        for (unsigned long long int i = 0; i < options->count; i++) {
            vmsl->send(msg_ctxt, content_loader);
        }
        
	
}