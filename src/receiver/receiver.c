/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "receiver.h"

static void messenger_function(log_level_t level, const char *msg, ...)
{
    const options_t *options = get_options_object();
    
    if (!can_log(level, options->log_level)) {
        return;
    }
    
    va_list ap;
    char *ret = NULL;

    va_start(ap, msg);
    vasprintf(&ret, msg, ap);
    va_end(ap);
    

    switch (level) {
    case TRACE:
        fprintf(stderr, "[TRACE]: %s\n", ret);
        break;
    case DEBUG:
        fprintf(stderr, "[DEBUG]: %s\n", ret);
        break;
    case INFO:
        fprintf(stderr, "[INFO]: %s\n", ret);
        break;
    case WARNING:
        fprintf(stderr, "[WARNING]: %s\n", ret);
        break;
    case ERROR:
        fprintf(stderr, "[ERROR]: %s\n", ret);
        break;
    case FATAL:
        fprintf(stderr, "[FATAL]: %s\n", ret);
        break;
    default:
        fprintf(stderr, "[MSG]: %s\n", ret);
        break;
    }

    
    free(ret);
    fflush(NULL);
}

static void show_help()
{
    printf("Usage: ");
    printf("\t-b\t--broker-url=<url> broker-url\n");
    printf("\t-d\t--debug runs in debug verbose mode\n");
    printf("\t-t\t--trace runs in trace verbose mode\n");

    printf("\t-L\t--logdir=<logdir> a directory to save the logs (mandatory for --daemon)\n");
    printf("\t-h\t--help show this help\n");
}

int main(int argc, char **argv)
{
    int c;
    int option_index = 0;

    options_t *options = options_new();

    if (!options) {
        return EXIT_FAILURE;
    }

    set_options_object(options);
    set_logger(messenger_function);
    while (1) {

        static struct option long_options[] = {
            { "broker-url", true, 0, 'b'},
            { "debug", false, 0, 'd'},
            { "daemon", false, 0, 'D'},
            { "trace", false, 0, 't'},
            { "logdir", true, 0, 'L'},
            { "help", false, 0, 'h'},
            { 0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "b:dDc:L:h", long_options, &option_index);
        if (c == -1) {
            if (optind == 1) {
                fprintf(stderr, "Not enough options\n");
                show_help();
                return EXIT_FAILURE;
            }
            break;
        }

        switch (c) {
        case 'b':
            strncpy(options->url, optarg, sizeof (options->url) - 1);
            break;
        case 'D':
            options->daemon = true;
            break;
        case 'l':
            options->log_level = get_log_level(optarg);
            break;
        case 'L':
            strncpy(options->logdir, optarg, sizeof (options->logdir) - 1);
            break;
        case 'h':
            show_help();
            return EXIT_SUCCESS;
        default:
            printf("Invalid or missing option\n");
            show_help();
            break;
        }
    }

    if (strlen(options->logdir) > 0) {
        // remap_log(options->logdir, "mpt-receiver", options->pid, stderr);
    }

    vmsl_t *vmsl = vmsl_init();
    vmsl->init = proton_init;
    vmsl->receive = proton_receive;
    vmsl->subscribe = proton_subscribe;
    
    receiver_start(vmsl, options);
    
    return EXIT_SUCCESS;
}