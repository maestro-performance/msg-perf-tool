/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "process_utils.h"

/**
 * Initializes the controller process if in daemon mode
 * @param options
 * @param logdir
 * @param controller_name
 */
void init_controller(bool daemon, const char *logdir, const char *controller_name)
{
    if (daemon) {
        int controller = fork();

        if (controller == 0) {
            setsid();

            int fd = open("/dev/null", O_RDWR, 0);

            if (fd != -1) {
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);

                if (fd > 2) {
                    close(fd);
                }
            }

            remap_log(logdir, controller_name, 0, getpid(), stderr);
        }
        else {
            if (controller > 0) {
                printf("%d\n", controller);
                exit(0);
            }
            else {
                fprintf(stderr, "Unable to create child process");
                exit(1);
            }

        }
    }
}