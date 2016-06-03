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
    
    msg_content_data_t content_storage; 
    
    content_storage.data = malloc(options->message_size);
    content_storage.capacity = options->message_size;
    while (true) {
        vmsl->receive(msg_ctxt, &content_storage);
    }

    free(content_storage.data);
}