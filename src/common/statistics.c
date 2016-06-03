/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "statistics.h"

/*
static mpt_timestamp_t proton_timestamp_to_mpt_timestamp_t(pn_timestamp_t timestamp) {
    mpt_timestamp_t ret = {0};
    
    double ts =  ((double) timestamp / 1000);
    logger_t logger = get_logger(); 
    
    logger(DEBUG, "Timestamp: %lu / %f", timestamp, ts);
    
    ret.tv_sec = abs(ts);
    
    double err;
    ret.tv_usec = modf(ts, &err) * 1000;
    
    logger(DEBUG, "Returning: %lu / %lu / %f", ret.tv_sec, ret.tv_usec, err);
    
    return ret;
}
 */

mpt_timestamp_t statistics_now() {
    struct timeval ret;
    
    gettimeofday(&ret, NULL);
    return ret;
}

unsigned long long statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end) {
    unsigned long long t1 = ( start.tv_sec) * 1000 + (start.tv_usec / 1000);
    unsigned long long t2 = ( end.tv_sec) * 1000 + (end.tv_usec / 1000);

    return t2 - t1;
}

void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end) {
    logger_t logger = get_logger();
    
    logger(INFO, "Creation time: %d.%d", start.tv_sec, start.tv_usec);
    logger(INFO, "End time: %d.%d", end.tv_sec, end.tv_usec);
    logger(INFO, "Latency: %d (secs)", (end.tv_sec - start.tv_sec));
}