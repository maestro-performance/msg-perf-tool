/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   process_utils.h
 * Author: opiske
 *
 * Created on June 6, 2016, 9:46 AM
 */

#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdbool.h>
#include <unistd.h>
    
#include "contrib/log.h"

void init_controller(bool daemon, const char *logdir, const char *controller_name);


#ifdef __cplusplus
}
#endif

#endif /* PROCESS_UTILS_H */

