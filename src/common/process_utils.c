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
#include "process_utils.h"


bool remap_log(const char *dir, const char *base_name, pid_t parent, 
                      pid_t pid, FILE *fd, gru_status_t *status)
{
    char name[64];

    bzero(name, sizeof (name));

    if (parent == 0) {
        snprintf(name, sizeof (name) - 1, "%s-%d.log", base_name, pid);
    }
    else {
        snprintf(name, sizeof (name) - 1, "%s-%d-%d.log", base_name, parent, pid);
    }

    return gru_io_remap(dir, name, fd, status);
}

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

            gru_status_t status = {0};
            
            bool ret = remap_log(logdir, controller_name, 0, getpid(), stderr, 
                                 &status);
            
            if (!ret) {
                fprintf(stderr, "Unable to initialize the controller: %s", 
                        status.message);
                exit(1);
            }
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