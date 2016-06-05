/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   statistics.h
 * Author: opiske
 *
 * Created on June 3, 2016, 11:57 AM
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "contrib/logger.h"
#include <time.h>
#include <sys/time.h>
    
typedef struct timeval mpt_timestamp_t;

void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end);
unsigned long long statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end);
mpt_timestamp_t statistics_now();


#ifdef __cplusplus
}
#endif

#endif /* STATISTICS_H */

