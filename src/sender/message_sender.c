/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_sender.h"

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

    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;

    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
}

static void install_interrupt_handler()
{
    struct sigaction sa;
    struct itimerval timer;

    memset(&sa, 0, sizeof (sa));

    sa.sa_handler = &interrupt_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &sa, NULL);
}

static bool can_continue(const options_t *options, unsigned long long int sent)
{
    struct timeval now;
    
    if (interrupted) {
        return false;
    }
    
    gettimeofday(&now, NULL);

    if ((sent < options->count) || options->count == 0) {
        if (now.tv_sec <= options->duration.tv_sec || options->duration.tv_sec == 0) {
            return true;
        }
    }
    
    return false;
}

static void content_loader(msg_content_data_t *content_data)
{
    content_data->capacity = 10;
    content_data->data = "0123456789";
}

void sender_start(const vmsl_t *vmsl, const options_t *options)
{
    logger_t logger = get_logger();
   
    msg_ctxt_t *msg_ctxt = vmsl->init(NULL);
    install_timer();
    install_interrupt_handler();

    mpt_timestamp_t last;
    mpt_timestamp_t start = statistics_now();

    register unsigned long long int sent = 0;
    time_t last_calc = 0;
    while (can_continue(options, sent)) {
        vmsl->send(msg_ctxt, content_loader);
        sent++;
        last = statistics_now();
        
        if (last_calc != last.tv_sec && (last.tv_sec % 10) == 0) {
            unsigned long long partial = statistics_diff(start, last);
            double rate = ((double) sent / partial) * 1000;
    
            logger(STAT, "count:%lu|duration:%lu|rate:%.2f msgs/sec", sent,
                partial, rate);
            last_calc = last.tv_sec;
        }
    }
    
    vmsl->stop(msg_ctxt);
    vmsl->destroy(msg_ctxt);

    unsigned long long elapsed = statistics_diff(start, last);
    double rate = ((double) sent / elapsed) * 1000;

    logger(STAT, "Sent %lu messages in %lu milliseconds: %.2f msgs/sec", sent,
           elapsed, rate);
}