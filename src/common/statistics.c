/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "statistics.h"


void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end) {
    logger_t logger = get_logger();
    
    logger(INFO, "Creation time: %d.%d", start.tv_sec, start.tv_usec);
    logger(INFO, "End time: %d.%d", end.tv_sec, end.tv_usec);
    logger(INFO, "Latency: %d (secs)", (end.tv_sec - start.tv_sec));
}