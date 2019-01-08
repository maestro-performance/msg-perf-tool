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
#ifndef MAESTRO_PLAYER_H
#define MAESTRO_PLAYER_H

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

#include <collection/gru_list.h>
#include <common/gru_alloc.h>
#include <common/gru_status.h>
#include <contrib/options.h>
#include <log/gru_logger.h>

#include "msg_conn_info.h"
#include "vmsl.h"

#include "maestro_player_info.h"
#include "maestro_sheet.h"
#include "maestro_topics.h"
#include "maestro_notify.h"

#define QOS_AT_MOST_ONCE 0
#define QOS_AT_LEAST_ONCE 1
#define QOS_EXACTLY_ONCE 2


#define MPT_MAESTRO_IDLE_TIME 10000

#ifdef __cplusplus
extern "C" {
#endif

extern bool vmsl_assign_by_url(const gru_uri_t *uri, vmsl_t *vmsl);

typedef struct maestro_player_t_ {
	pthread_t thread;
	gru_uri_t uri;
	vmsl_t mmsl; // maestro messaging system layer
  	vmslh_handlers_t handlers;

	msg_ctxt_t *ctxt; // maestro messaging context
	maestro_sheet_t *sheet;
	maestro_player_info_t player_info;

	bool cancel;
} maestro_player_t;

bool maestro_player_start(maestro_sheet_t *sheet, gru_status_t *status);
bool maestro_player_stop(maestro_sheet_t *sheet, gru_status_t *status);

const maestro_player_t *maestro_player();

#ifdef __cplusplus
}
#endif

#endif /* MAESTRO_PLAYER_H */
