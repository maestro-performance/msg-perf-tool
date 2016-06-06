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
void init_controller(bool daemon, const char *logdir, const char *controller_name) {
    if (daemon) {
        int controller = fork();
        
        if (controller == 0) {
            remap_log(logdir, controller_name, 0, getpid(), stderr);
        }
        else {
            printf("%d\n", controller);
            exit(0);
        }
    }
}