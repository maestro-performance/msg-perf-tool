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
#ifndef MSGCTX_H
#define MSGCTX_H

#include <stdint.h>
#include <stdlib.h>

#include <common/gru_alloc.h>
#include <log/gru_logger.h>
#include <network/gru_uri.h>

#include "msg_conn_info.h"
#include "msg_content_data.h"
#include "msg_direction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*msg_content_loader)(msg_content_data_t *content_data);

typedef enum msg_stat_opt_t_ {
	MSG_STAT_NONE = 0, /** Disable statistics **/
	MSG_STAT_LATENCY = 1, /** Enable latency statistics **/
	MSG_STAT_THROUGHPUT = 2, /** Enable throughtput statistics **/
	MSG_STAT_DEFAULT = (MSG_STAT_LATENCY | MSG_STAT_THROUGHPUT)
} msg_stat_opt_t;

/**
 * Messaging options
 */
typedef struct msg_opt_t_ {
	msg_direction_t direction;
	msg_stat_opt_t statistics;
	msg_conn_info_t conn_info;
	gru_uri_t uri;
} msg_opt_t;

typedef struct msg_ctxt_t_ {
	void *api_context;
	msg_opt_t msg_opts;
} msg_ctxt_t;

msg_ctxt_t *msg_ctxt_init(gru_status_t *status);
void msg_ctxt_destroy(msg_ctxt_t **ctxt);

#ifdef __cplusplus
}
#endif

#endif /* MSGCTX_H */
