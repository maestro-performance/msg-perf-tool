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
#include "contrib/options.h"

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

static FILE *open_stats_file(const char *prefix, const char *suffix,
                             gru_status_t *status) {
    const options_t *options = get_options_object();
    char name[64] = {0};

    snprintf(name, sizeof (name) - 1, "%s-%s-%d.csv", prefix, suffix, getpid());

    return gru_io_open_unique_file(options->logdir, name, status);
}

static FILE *open_receiver_latency_file(gru_status_t *status) {
    return open_stats_file("receiver", "latency", status);
}

static FILE *open_receiver_throughput_file(gru_status_t *status) {
    return open_stats_file("receiver", "throughput", status);
}
static FILE *open_sender_throughput_file(gru_status_t *status) {
    return open_stats_file("sender", "throughput", status);
}

stat_io_t *statistics_init(stat_direction_t direction, gru_status_t *status) {
    logger_t logger = gru_logger_get();
    stat_io_t *ret = malloc(sizeof(stat_io_t));

    if (direction == SENDER) {
        ret->latency = NULL;
        ret->throughput = open_sender_throughput_file(status);
    }
    else {
        ret->latency = open_receiver_latency_file(status);
        ret->throughput = open_receiver_throughput_file(status);
    }

    if (!ret->throughput) {
        logger(ERROR, "Unable to initialize the statistics IO engine: %s",
               status->message);

        goto err_exit;
    }

    if (direction == RECEIVER) {
        if (!ret->latency) {
            logger(ERROR, "Unable to initialize the statistics IO engine: %s",
                   status->message);

            goto err_exit;
        }
    }

    ret->direction = direction;
    return ret;

    err_exit:
    statistics_destroy(&ret);
    return NULL;
}


stat_io_t *statistics_init_stdout(stat_direction_t direction, gru_status_t *status) {
    stat_io_t *ret = malloc(sizeof(stat_io_t));

    if (direction == SENDER) {
        ret->latency = NULL;
        ret->throughput = stdout;
    }
    else {
        ret->latency = stdout;
        ret->throughput = stdout;
    }

    ret->direction = direction;
    return ret;
}


void statistics_destroy(stat_io_t **stat_io) {
    if ((*stat_io)->latency) {
        fclose((*stat_io)->latency);
    }

    if ((*stat_io)->throughput) {
        fclose((*stat_io)->throughput);
    }

    free(*stat_io);
    *stat_io = NULL;
}

void statistics_latency_header(stat_io_t *stat_io) {
    fprintf(stat_io->latency, "creation;latency\n");
}

void statistics_throughput_header(stat_io_t *stat_io) {
    fprintf(stat_io->throughput, "timestamp;count;duration;rate\n");
}

void statistics_latency_data(stat_io_t *stat_io, uint64_t latency,
                             const char *time, int32_t milli)
{
    fprintf(stat_io->latency, "%s.%03"PRId32";%"PRIu64"\n", time, milli, latency);
}


void statistics_throughput_data(stat_io_t *stat_io, const char *last_buff,
                                uint64_t count, uint64_t partial, double rate) {
    fprintf(stat_io->throughput, "%s;%"PRIu64";%"PRIu64";%.2f\n", last_buff, count, partial, rate);
}

static uint64_t statistics_convert_to_milli(mpt_timestamp_t ts) {
    return (((ts.tv_sec) * 1000) + (ts.tv_usec / 1000));

}


uint64_t statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end)
{
    mpt_timestamp_t ret = {.tv_sec = 0, .tv_usec = 0};
    timersub(&end, &start, &ret);

    logger_t logger = gru_logger_get();

    /*
     * At least until I have time to dig further into this, this may be entirely
     * wishful thinking on 32 bits platforms, since I am not sure that this will
     * work in all the cases or whether this is safe at all.
     */
    logger(DEBUG, "Calculated diff : %"PRIi64".%"PRIi64"",
           (int64_t) ret.tv_sec, (int64_t) ret.tv_usec);

    return statistics_convert_to_milli(ret);
}

void statistics_latency(stat_io_t *stat_io, mpt_timestamp_t start, mpt_timestamp_t end)
{
    logger_t logger = gru_logger_get();

    logger(DEBUG, "Creation time: %"PRIi64".%"PRIi64"",
           (int64_t) start.tv_sec, (int64_t) start.tv_usec);
    logger(DEBUG, "Received time: %"PRIi64".%"PRIi64"",
           (int64_t) end.tv_sec, (int64_t) end.tv_usec);

    char tm_creation_buff[64] = {0};

    struct tm result;
    struct tm *creation_tm = localtime_r(&start.tv_sec, &result);

    if (!creation_tm) {
        logger(ERROR, "Unable to calculate current localtime");

        return;
    }

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
        statistics_latency_data(stat_io, statistics_diff(start, end),
                                tm_creation_buff, (start.tv_usec/1000));
    }
}

void statistics_throughput_partial(stat_io_t *stat_io, mpt_timestamp_t start,
                                   mpt_timestamp_t last, uint64_t count) {
    uint64_t partial = statistics_diff(start, last);
    double rate = ((double) count / partial) * 1000;

    char last_buff[64] = {0};

    struct tm *last_tm = localtime(&last.tv_sec);
    strftime(last_buff, sizeof(last_buff), "%Y-%m-%d %H:%M:%S", last_tm);

    statistics_throughput_data(stat_io, last_buff, count, partial, rate);
}