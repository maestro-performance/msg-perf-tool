/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "statistics.h"

#ifndef __linux__

/*
 * Timer subtraction, highly inspired on the code show at: 
 * http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 */

static int timersub(struct timeval *start, struct timeval *end, struct timeval *result)
{
    if (start->tv_usec < end->tv_usec) {
        int nsec = (end->tv_usec - start->tv_usec) / 1000000 + 1;
        end->tv_usec -= 1000000 * nsec;
        end->tv_sec += nsec;
    }
    if (start->tv_usec - end->tv_usec > 1000000) {
        int nsec = (start->tv_usec - end->tv_usec) / 1000000;
        end->tv_usec += 1000000 * nsec;
        end->tv_sec -= nsec;
    }
    
    result->tv_sec = start->tv_sec - end->tv_sec;
    result->tv_usec = start->tv_usec - end->tv_usec;
    
    return start->tv_sec < end->tv_sec;
}

#endif // __linux__




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
    logger(STAT, "latency:%llu|creation:%llu|received:%llu", 
           statistics_diff(start, end), start.tv_sec, end.tv_sec);

}