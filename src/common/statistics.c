/**
 Copyright 2016 Otavio Rodolfo Piske
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
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


static uint64_t statistics_convert_to_milli(mpt_timestamp_t ts) {
    return (((ts.tv_sec) * 1000) + (ts.tv_usec / 1000));
    
}


uint64_t statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end)
{
    mpt_timestamp_t ret = {.tv_sec = 0, .tv_usec = 0};
    timersub(&end, &start, &ret);
    
    logger_t logger = get_logger();
    
    /*
     * At least until I have time to dig further into this, this may be entirely
     * wishful thinking on 32 bits platforms, since I am not sure that this will
     * work in all the cases or whether this is safe at all.
     */
    logger(DEBUG, "Calculated diff : %"PRIi64".%"PRIi64"", 
           (int64_t) ret.tv_sec, (int64_t) ret.tv_usec);
    
    return statistics_convert_to_milli(ret);
}

void statistics_latency(mpt_timestamp_t start, mpt_timestamp_t end)
{
    logger_t logger = get_logger();

    logger(DEBUG, "Creation time: %"PRIi64".%"PRIi64"", 
           (int64_t) start.tv_sec, (int64_t) start.tv_usec);
    logger(DEBUG, "Received time: %"PRIi64".%"PRIi64"", 
           (int64_t) end.tv_sec, (int64_t) end.tv_usec);
    
    char tm_creation_buff[64] = {0};
    
    struct tm *creation_tm = localtime(&start.tv_sec);
    strftime(tm_creation_buff, sizeof(tm_creation_buff), "%Y-%m-%d %H:%M:%S", 
             creation_tm);
    
    if (start.tv_sec == 0) {
        char tm_received_buff[64] = {0};
        
        struct tm *received_tm = localtime(&end.tv_sec);
        strftime(tm_received_buff, sizeof(tm_received_buff), 
                 "%Y-%m-%d %H:%M:%S", received_tm);
    
        logger(STAT, "error;%"PRIu64";creation;%s.%"PRId32";received;%s.%"PRId32"",
           statistics_diff(start, end), tm_creation_buff, (start.tv_usec/1000),
                tm_received_buff, (end.tv_usec/1000));
    }
    else {
        logger(STAT, "latency;%"PRIu64";creation;%s.%"PRId32"",
           statistics_diff(start, end), tm_creation_buff, (start.tv_usec/1000));
    }
    
    
}