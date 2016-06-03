/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "statistics.h"

mpt_timestamp_t statistics_now()
{
    struct timeval ret = {.tv_sec = 0, .tv_usec = 0};

    gettimeofday(&ret, NULL);
    return ret;
}


unsigned long long statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end)
{
    mpt_timestamp_t ret = {.tv_sec = 0, .tv_usec = 0};
    timersub(&end, &start, &ret);
    
    logger_t logger = get_logger();
    
    // logger(DEBUG, "Start time: %d.%d", start.tv_sec, start.tv_usec);
    // logger(DEBUG, "End time: %d.%d", end.tv_sec, end.tv_usec);
    logger(DEBUG, "Calculated diff : %ld.%ld", ret.tv_sec, ret.tv_usec);
    return (((ret.tv_sec) * 1000) + (ret.tv_usec / 1000));
}

void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end)
{
    logger_t logger = get_logger();

    logger(DEBUG, "Creation time: %d.%d", start.tv_sec, start.tv_usec);
    logger(DEBUG, "Received time: %d.%d", end.tv_sec, end.tv_usec);
    logger(STAT, "latency,%llu,milliseconds", statistics_diff(start, end));

}