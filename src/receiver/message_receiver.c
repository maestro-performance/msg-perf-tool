/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_receiver.h"
#include "vmsl.h"

static bool interrupted = false;

static void timer_handler(int signum)
{
    logger_t logger = get_logger();
    
    logger(TRACE, "Activity timer expired");
}


static void interrupt_handler(int signum)
{
    logger_t logger = get_logger();
    
    logger(TRACE, "Interrupted");
    interrupted = true;
}

static void install_timer()
{
    struct sigaction sa;
    struct itimerval timer;

    memset(&sa, 0, sizeof (sa));

    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 30;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 30;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
}

static void install_interrupt_handler()
{
    struct sigaction sa;

    memset(&sa, 0, sizeof (sa));

    sa.sa_handler = &interrupt_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &sa, NULL);
}

static bool can_continue(const options_t *options)
{
    struct timeval now;
    
    if (interrupted) {
        return false;
    }

    if (options->duration.tv_sec == 0) {
        return true;
    }

    gettimeofday(&now, NULL);

    if (now.tv_sec >= options->duration.tv_sec) {
        return false;
    }

    return true;
}

void receiver_start(const vmsl_t *vmsl, const options_t *options)
{
    logger_t logger = get_logger();
    msg_ctxt_t *msg_ctxt = vmsl->init(NULL);
 

    install_timer();
    install_interrupt_handler();
            
    vmsl->subscribe(msg_ctxt, NULL);

    msg_content_data_t content_storage;

    content_storage.data = malloc(options->message_size);
    bzero(content_storage.data, options->message_size);
    content_storage.capacity = options->message_size;
    content_storage.count = 0;

    mpt_timestamp_t last;
    mpt_timestamp_t start = statistics_now();
    time_t last_calc = 0;
    while (can_continue(options)) {
        vmsl->receive(msg_ctxt, &content_storage);
        last = statistics_now();
        
        if (last_calc != last.tv_sec && (last.tv_sec % 10) == 0) {
            unsigned long long partial = statistics_diff(start, last);
            double rate = ((double) content_storage.count / partial) * 1000;
            
            char last_buff[64] = {0};
            
            struct tm *last_tm = localtime(&last.tv_sec);
            strftime(last_buff, sizeof(last_buff), "%Y-%m-%d %H:%M:%S", last_tm);
    
            logger(STAT, "ts;%s;count;%lu;duration;%llu;rate;%.2f", last_buff,
                   content_storage.count, partial, rate);
            
            last_calc = last.tv_sec;
        }
    }   

    unsigned long long elapsed = statistics_diff(start, last);
    double rate = ((double) content_storage.count / elapsed) * 1000;
    logger(INFO, "Summary: received %lu messages in %llu milliseconds (rate: %.2f msgs/sec)",
           (content_storage.count - 1),
           elapsed, rate);

    free(content_storage.data);
}