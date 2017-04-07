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
#include <common/gru_status.h>
#include <log/gru_logger.h>
#include <contrib/options.h>

#include "vmsl.h"

extern bool vmsl_assign_by_url(gru_uri_t *uri, vmsl_t *vmsl);

typedef struct maestro_player_t_ {
	pthread_t thread;
	gru_uri_t uri;
	vmsl_t mmsl; // maestro messaging system layer
	msg_ctxt_t *ctxt; // maestro messaging context

	bool cancel;
} maestro_player_t;

maestro_player_t maestro_player_new() {
	maestro_player_t ret = {0};

	ret.mmsl = vmsl_init();
	
	return ret;
}

bool maestro_player_start(const options_t *options, gru_status_t *status);

#endif /* MAESTRO_PLAYER_H */
