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
#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <common/gru_status.h>
#include <io/gru_ioutils.h>
#include <log/gru_logger.h>

#include "contrib/options.h"

#ifdef __cplusplus
extern "C" {
#endif

bool remap_log(const char *dir, const char *base_name, pid_t parent, pid_t pid, FILE *fd,
	gru_status_t *status);
void init_controller(bool daemon, const char *logdir, const char *controller_name);

bool can_continue(const options_t *options, uint64_t sent);

void install_timer(time_t sec);
void install_interrupt_handler();

/**
 * Does the obvious: creates a POSIX queue
 * @param key
 * @return
 */
int create_queue(key_t key, gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* PROCESS_UTILS_H */
