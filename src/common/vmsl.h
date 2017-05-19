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
#ifndef VMSL_H
#define VMSL_H

#include <common/gru_status.h>
#include <log/gru_logger.h>

#include "msgctxt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vmsl_stat_t_ {
	VMSL_ERROR = 0,
	VMSL_SUCCESS = 1,
	VMSL_NO_DATA = 2,
	VMSL_NO_TIMESTAMP = 4,
} vmsl_stat_t;

typedef msg_ctxt_t *(*msg_init)(msg_opt_t opt, void *data, gru_status_t *status);
typedef vmsl_stat_t (
	*msg_send)(msg_ctxt_t *ctxt, msg_content_data_t *data, gru_status_t *status);
typedef vmsl_stat_t (*msg_subscribe)(msg_ctxt_t *ctxt, void *data, gru_status_t *status);
typedef vmsl_stat_t (
	*msg_receive)(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status);
typedef void (*msg_stop)(msg_ctxt_t *ctxt, gru_status_t *status);
typedef void (*msg_destroy)(msg_ctxt_t *, gru_status_t *status);

typedef struct vmsl_t_ {
	msg_init init;
	msg_send send;
	msg_subscribe subscribe;
	msg_receive receive;
	msg_stop stop;
	msg_destroy destroy;
} vmsl_t;

vmsl_t vmsl_init();

static inline bool vmsl_stat_success(vmsl_stat_t stat) {
	return (stat & VMSL_SUCCESS) ? true : false;
}

static inline bool vmsl_stat_error(vmsl_stat_t stat) {
	return (stat & VMSL_SUCCESS) ? false : true;
}

#ifdef __cplusplus
}
#endif

#endif /* VMSL_H */
