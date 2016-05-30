/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vmsl.h
 * Author: otavio
 *
 * Created on November 1, 2015, 11:01 AM
 */

#ifndef VMSL_H
#define VMSL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*msg_init)(void *data); 
typedef void(*msg_send)(void *data);
typedef void(*msg_commit)(void *data);

typedef struct vmsl_t_ {
	msg_init init;
        msg_send send;
        msg_commit commit;
} vmsl;


#ifdef __cplusplus
}
#endif

#endif /* VMSL_H */

