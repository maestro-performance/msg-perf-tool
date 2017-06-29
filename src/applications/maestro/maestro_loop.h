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
#ifndef MAESTRO_LOOP_H
#define MAESTRO_LOOP_H

#include <stdint.h>
#include <stdio.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <common/gru_colors.h>

#include "maestro_cmd_ctxt.h"
#include "maestro_command.h"

#ifdef __cplusplus
extern "C" {
#endif

int maestro_loop(gru_status_t *status);
void maestro_loop_reply(const options_t *options, gru_status_t *status);

#ifdef __cplusplus
}
#endif


#endif /* MAESTRO_LOOP_H */
