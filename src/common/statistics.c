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

static unsigned long long statistics_convert_to_milli(mpt_timestamp_t ts) {
    return (((ts.tv_sec) * 1000) + (ts.tv_usec / 1000));
    
}


unsigned long long statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end)
{
    mpt_timestamp_t ret = {.tv_sec = 0, .tv_usec = 0};
    timersub(&end, &start, &ret);
    
    logger_t logger = get_logger();
    
    logger(DEBUG, "Calculated diff : %ld.%ld", ret.tv_sec, ret.tv_usec);
    
    return statistics_convert_to_milli(ret);
}

void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end)
{
    logger_t logger = get_logger();

    logger(DEBUG, "Creation time: %d.%d", start.tv_sec, start.tv_usec);
    logger(DEBUG, "Received time: %d.%d", end.tv_sec, end.tv_usec);
    
    char tm_creation_buff[64] = {0};
    
    struct tm *creation_tm = localtime(&start.tv_sec);
    strftime(tm_creation_buff, sizeof(tm_creation_buff), "%Y-%m-%d %H:%M:%S", creation_tm);
    
    if (start.tv_sec == 0) {
        char tm_received_buff[64] = {0};
        
        struct tm *received_tm = localtime(&end.tv_sec);
        strftime(tm_received_buff, sizeof(tm_received_buff), "%Y-%m-%d %H:%M:%S", received_tm);
    
        logger(STAT, "error;%llu;creation;%s.%d;received;%s.%d",
           statistics_diff(start, end), tm_creation_buff, (start.tv_usec/1000),
                tm_received_buff, (end.tv_usec/1000));
    }
    else {
        logger(STAT, "latency;%llu;creation;%s.%d",
           statistics_diff(start, end), tm_creation_buff, (start.tv_usec/1000));
    }
    
    
}