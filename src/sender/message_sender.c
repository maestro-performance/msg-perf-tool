/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_sender.h"

static void content_loader(msg_content_data_t *content_data)
{
    content_data->capacity = 10;
    content_data->data = "0123456789";
}

void sender_start(const vmsl_t *vmsl, const options_t *options)
{
    logger_t logger = get_logger();

    msg_ctxt_t *msg_ctxt = vmsl->init(NULL);

    mpt_timestamp_t start = statistics_now();
    
    for (unsigned long long int i = 0; i < options->count; i++) {
        vmsl->send(msg_ctxt, content_loader);
    }
    mpt_timestamp_t end  = statistics_now();
    
    vmsl->stop(msg_ctxt);
    vmsl->destroy(msg_ctxt);

    logger(INFO, "Sent %lu messages in %lu milliseconds", options->count, 
           statistics_diff(start, end));
}