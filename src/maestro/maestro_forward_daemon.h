/**
 *    Copyright 2017 Otavio Rodolfo Piske
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef MAESTRO_FORWARD_DAEMON_H
#define MAESTRO_FORWARD_DAEMON_H

#include <stdint.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <common/gru_colors.h>

#include "maestro_cmd_ctxt.h"
#include "maestro_command.h"

int maestro_forward_daemon_run(const options_t *options);

#endif /* MAESTRO_FORWARDD_H_DAEMON*/
