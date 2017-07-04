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
#ifndef DAEMON_COMMON_H
#define DAEMON_COMMON_H

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <common/gru_status.h>

#include "contrib/options.h"
#include "maestro/maestro_player.h"
#include "maestro/maestro_player.h"
#include "maestro/maestro_sheet.h"
#include "maestro/maestro_sheet.h"

#include "worker_options.h"
#include "worker_types.h"
#include "worker_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void *commond_handle_set(const maestro_note_t *request, maestro_note_t *response,
	worker_options_t *worker_options);
void *commond_handle_flush(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo);
void *commond_handle_ping(const maestro_note_t *request, maestro_note_t *response,
	const maestro_player_info_t *pinfo);

#ifdef __cplusplus
}
#endif


#endif /* DAEMON_COMMON_H */
