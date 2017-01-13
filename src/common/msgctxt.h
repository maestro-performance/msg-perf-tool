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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#include <log/gru_logger.h>
#include <common/gru_alloc.h>

#include "statistics.h"


typedef struct msg_content_data_t_ {
	uint64_t count;
	uint64_t errors;
	size_t capacity;
	size_t size;
	void *data;
} msg_content_data_t;

typedef void (*msg_content_loader)(msg_content_data_t *content_data);

typedef enum msg_direction_t_ {
    MSG_DIRECTION_SENDER,
    MSG_DIRECTION_RECEIVER
} msg_direction_t;

typedef enum msg_qos_t_ {
    MSG_QOS_AT_MOST_ONCE,
    MSG_QOS_AT_LEAST_ONCE, /** Not fully supported and reserved for future use **/
    MSG_QOS_EXACTLY_ONCE, /** Not fully supported and reserved for future use **/
} msg_qos_t;

/**
 * Messaging options
 */
typedef struct msg_opt_t_ {
    msg_direction_t direction;
    msg_qos_t qos;
} msg_opt_t;


typedef struct msg_ctxt_t_ {
	void *api_context;
	stat_io_t *stat_io;
        msg_opt_t msg_opts;
} msg_ctxt_t;

msg_ctxt_t *msg_ctxt_init(stat_io_t *stat_io, gru_status_t *status);
void msg_ctxt_destroy(msg_ctxt_t **ctxt);

#ifdef __cplusplus
}
#endif

#endif /* MSGCTX_H */
