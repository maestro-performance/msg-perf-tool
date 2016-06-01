/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_sender.h"


void sender_start(const vmsl_t *vmsl, const options_t *options) {
	msg_ctxt_t *msg_ctxt = vmsl->init(NULL);
	
	vmsl->send(msg_ctxt, NULL);
}