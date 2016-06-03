/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_receiver.h"
#include "vmsl.h"

void receiver_start(const vmsl_t *vmsl, const options_t *options)
{
    logger_t logger = get_logger();
    msg_ctxt_t *msg_ctxt = vmsl->init(NULL);

    vmsl->subscribe(msg_ctxt, NULL);
    
    msg_content_data_t content_storage; 
    
    content_storage.data = malloc(options->message_size);
    content_storage.capacity = options->message_size;
    content_storage.count = 0;
    
    mpt_timestamp_t start = statistics_now();
    while (true) {
        vmsl->receive(msg_ctxt, &content_storage);
    }
    mpt_timestamp_t end  = statistics_now();
    
    logger(INFO, "Received %lu messages in %lu milliseconds", 
           content_storage.count, 
           statistics_diff(start, end));

    free(content_storage.data);
}