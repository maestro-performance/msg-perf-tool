/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "message_sender.h"

static void timer_handler(int signum)
{
    logger_t logger = get_logger();
    
    logger(TRACE, "Activity timer expired");
}

static void install_timer()
{
    struct sigaction sa;
    struct itimerval timer;

    memset(&sa, 0, sizeof (sa));

    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec... */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;

    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    /* Start a virtual timer. It counts down whenever this process is
      executing. */
    setitimer(ITIMER_REAL, &timer, NULL);

}

static bool can_continue(const options_t *options, unsigned long long int sent)
{
    struct timeval now;
    
    gettimeofday(&now, NULL);

    if (sent < options->count) {
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

    mpt_timestamp_t start = statistics_now();

    register unsigned long long int sent = 0;
    while (can_continue(options, sent)) {
        vmsl->send(msg_ctxt, content_loader);
        sent++;
    }
    mpt_timestamp_t end = statistics_now();

    vmsl->stop(msg_ctxt);
    vmsl->destroy(msg_ctxt);

    unsigned long long elapsed = statistics_diff(start, end);
    double rate = ((double) sent / elapsed) * 1000;

    logger(INFO, "Sent %lu messages in %lu milliseconds: %f msgs/sec", sent,
           elapsed, rate);
}