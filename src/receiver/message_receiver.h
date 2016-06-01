/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   message_receiver.h
 * Author: opiske
 *
 * Created on June 1, 2016, 2:10 PM
 */

#ifndef MESSAGE_RECEIVER_H
#define MESSAGE_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmsl.h"
#include "msgctxt.h"
#include "contrib/options.h"

void receiver_start(const vmsl_t *vmsl, const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_RECEIVER_H */

