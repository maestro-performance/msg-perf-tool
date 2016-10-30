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
#include "sender_tune_main.h"

static void show_help()
{
    printf("Usage: ");
    printf("\t-b\t--broker-url=<url> broker-url\n");
    printf("\t-c\t--count=<value> sends a fixed number of messages\n");
    printf("\t-l\t--log-level=<level> runs in the given verbose (info, stat, debug, etc) level mode\n");
    printf("\t-d\t--duration=<value> runs for a fixed amount of time (in minutes)\n");
    printf("\t-s\t--size=<value> message size (in bytes)\n");
    printf("\t-L\t--logdir=<logdir> a directory to save the logs (mandatory for --daemon)\n");
    printf("\t-h\t--help show this help\n");
}

static bool init_vmsl_proton(vmsl_t *vmsl)
{
    logger_t logger = gru_logger_get();

#ifdef __AMQP_SUPPORT__
    logger(INFO, "Initializing AMQP protocol");

    vmsl->init = proton_init;
    vmsl->send = proton_send;
    vmsl->stop = proton_stop;
    vmsl->destroy = proton_destroy;

    return true;
#else
    logger(ERROR, "AMQP protocol support was is not enabled");
    return false;
#endif // __AMQP_SUPPORT__
}

static bool init_vmsl_stomp(vmsl_t *vmsl)
{
    logger_t logger = gru_logger_get();

#ifdef __STOMP_SUPPORT__
    logger(INFO, "Initializing STOMP protocol");

    vmsl->init = litestomp_init;
    vmsl->send = litestomp_send;
    vmsl->stop = litestomp_stop;
    vmsl->destroy = litestomp_destroy;

    return true;
#else
    logger(ERROR, "STOMP protocol support was is not enabled");
    return false;
#endif // __STOMP_SUPPORT__
}

int tune_main(int argc, char **argv)
{
    int c = 0;
    int option_index = 0;
    
    if (argc < 2) {
        show_help();
        
        return EXIT_FAILURE;
    }

    options_t *options = options_new();

    if (!options) {
        return EXIT_FAILURE;
    }

    set_options_object(options);
        
    const char *apphome = gru_base_app_home("mpt");
    config_init(options, apphome, "mpt-sender.ini");
    
    gru_logger_set(gru_logger_default_printer);
    
    while (1) {

        static struct option long_options[] = {
            { "broker-url", true, 0, 'b'},
            { "count", true, 0, 'c'},
            { "log-level", true, 0, 'l'},
            { "duration", true, 0, 'd'},
            { "size", true, 0, 's'},
            { "logdir", true, 0, 'L'},
            { "help", false, 0, 'h'},
            { 0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "b:c:l:d:s:L:h", long_options, &option_index);
        if (c == -1) {
            if (optind == 1) {
                // Will use defaults from the configuration file
                break;
            }
            break;
        }

        switch (c) {
        case 'b':
            strncpy(options->url, optarg, sizeof (options->url) - 1);
            break;
        case 'c':
            options->count = strtol(optarg, NULL, 10);
            break;
        case 'l':
            options->log_level = gru_logger_get_level(optarg);
            break;
        case 'd':
            options->duration = gru_duration_from_minutes(atoi(optarg));
            break;
        case 's':
            options->message_size = atoi(optarg);
            break;
        case 'L':
            strncpy(options->logdir, optarg, sizeof (options->logdir) - 1);
            break;
        case 'h':
            show_help();
            free(apphome);
            return EXIT_SUCCESS;
        default:
            printf("Invalid or missing option\n");
            show_help();
            free(apphome);
            return EXIT_FAILURE;
        }
    }


    vmsl_t *vmsl = vmsl_init();

    if (strncmp(options->url, "amqp://", 7) == 0) {
        if (!init_vmsl_proton(vmsl)) {
            goto err_exit;
        }
    }
    else {
        if (strncmp(options->url, "stomp://", 8) == 0) {
            if (!init_vmsl_stomp(vmsl)) {
                goto err_exit;
            }
        }
    }

    logger_t logger = gru_logger_get();
    
    tune_start(vmsl, options);
    
    logger(INFO, "Tune execution terminated successfully");
    
success_exit:
    vmsl_destroy(&vmsl);
    options_destroy(&options);
    free(apphome);
    return EXIT_SUCCESS;

err_exit:
    vmsl_destroy(&vmsl);
    options_destroy(&options);
    free(apphome);
    return EXIT_FAILURE;
}
