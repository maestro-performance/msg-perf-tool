/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "receiver.h"

static void show_help()
{
    printf("Usage: ");
    printf("\t-b\t--broker-url=<url> broker-url\n");
    printf("\t-d\t--debug runs in debug verbose mode\n");
    printf("\t-t\t--trace runs in trace verbose mode\n");

    printf("\t-L\t--logdir=<logdir> a directory to save the logs (mandatory for --daemon)\n");
    printf("\t-h\t--help show this help\n");
}

static struct timeval get_duration(int count) {
    struct timeval ret; 
    
    gettimeofday(&ret, NULL);
    
    ret.tv_sec = ret.tv_sec + (count * 60);
    
    return ret;
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
    set_logger(default_logger);
    
    options->parallel_count = 1;
    while (1) {

        static struct option long_options[] = {
            { "broker-url", true, 0, 'b'},
            { "duration", true, 0, 'd'},
            { "daemon", false, 0, 'D'},
            { "log-level", true, 0, 'l'},
            { "parallel-count", true, 0, 'p'},
            { "message-size", true, 0, 's'},
            { "logdir", true, 0, 'L'},
            { "help", false, 0, 'h'},
            { 0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "b:d:l:p:s:Dc:L:h", long_options, &option_index);
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
        case 'd':
            options->duration = get_duration(atoi(optarg));
            break;
        case 'l':
            options->log_level = get_log_level(optarg);
            break;
        case 'p':
            options->parallel_count = atoi(optarg);
            break;
        case 's':
            options->message_size = atoll(optarg);
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

    

    vmsl_t *vmsl = vmsl_init();
    vmsl->init = proton_init;
    vmsl->receive = proton_receive;
    vmsl->subscribe = proton_subscribe;
    
    
    int childs[5];
    int child = 0; 
    logger_t logger = get_logger(); 
    
    if (options->parallel_count > 1) { 
        logger(INFO, "Creating %d concurrent operations", options->parallel_count);
        for (int i = 0; i < options->parallel_count; i++) { 
                child = fork(); 

                if (child == 0) {
                    if (strlen(options->logdir) > 0) {
                        remap_log(options->logdir, "mpt-receiver", getppid(), 
                                  getpid(), stderr);
                     }
                     
                     receiver_start(vmsl, options);
                     return 0; 
                }
                else {
                    if (child > 0) {
                            childs[i] = child;

                    }
                    else {
                            printf("Error\n");
                    }
                }
        }

        if (child > 0) { 
             int status = 0;
                for (int i = 0; i < options->parallel_count; i++) {
                    waitpid(childs[i], &status, 0);

                    logger(INFO, "Child process %d terminated with status %d", childs[i], status);
            }
        }
    }
    else {
        remap_log(options->logdir, "mpt-receiver", getpid(), 
                                  getpid(), stderr);
        
        receiver_start(vmsl, options);
    }

    vmsl_destroy(&vmsl);
    
    logger(INFO, "Test execution with parent ID %d terminated successfully\n", getpid());
    return EXIT_SUCCESS;
}