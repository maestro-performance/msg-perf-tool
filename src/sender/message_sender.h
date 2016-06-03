/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   message_sender.h
 * Author: opiske
 *
 * Created on June 1, 2016, 9:46 AM
 */

#ifndef MESSAGE_SENDER_H
#define MESSAGE_SENDER_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "vmsl.h"
#include "msgctxt.h"
#include "statistics.h"
#include "contrib/options.h"

void sender_start(const vmsl_t *vmsl, const options_t *options);


#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_SENDER_H */

