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
#include "vmslh.h"

#define MPT_STR_HELPER(x) #x
#define MPT_STR(x) MPT_STR_HELPER(x)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vmsl_stat_t_ {
	VMSL_ERROR = 0,
	VMSL_SUCCESS = 1,
	VMSL_NO_DATA = 2,
	VMSL_NO_TIMESTAMP = 4,
} vmsl_stat_t;

typedef struct vmsl_mtopic_spec_t_ {
	int count;
  	int qos;
  	char **topics;
} vmsl_mtopic_spec_t;

typedef struct vmsl_info_t_ {
  const char *api_name;
  const char *api_version;
} vmsl_info_t;

typedef msg_ctxt_t *(*msg_init)(msg_opt_t opt, vmslh_handlers_t *handlers, gru_status_t *status);
typedef vmsl_stat_t (*msg_start)(msg_ctxt_t *ctxt, gru_status_t *status);
typedef vmsl_stat_t (
	*msg_send)(msg_ctxt_t *ctxt, msg_content_data_t *data, gru_status_t *status);
typedef vmsl_stat_t (*msg_subscribe)(msg_ctxt_t *ctxt, vmsl_mtopic_spec_t *mtopic, gru_status_t *status);
typedef vmsl_stat_t (
	*msg_receive)(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status);
typedef void (*msg_stop)(msg_ctxt_t *ctxt, gru_status_t *status);
typedef void (*msg_destroy)(msg_ctxt_t *, gru_status_t *status);
typedef vmsl_info_t (*msg_info)(void);



typedef struct vmsl_t_ {
	msg_init init;
  	msg_start start;
	msg_send send;
	msg_subscribe subscribe;
	msg_receive receive;
	msg_stop stop;
	msg_destroy destroy;
	msg_info info;
} vmsl_t;

vmsl_t vmsl_init();

static inline bool vmsl_stat_success(vmsl_stat_t stat) {
	return (stat & VMSL_SUCCESS) ? true : false;
}

static inline bool vmsl_stat_error(vmsl_stat_t stat) {
	return (stat & VMSL_SUCCESS) ? false : true;
}

static inline bool vmsl_has_data(vmsl_stat_t stat) {
	return (stat & VMSL_NO_DATA) ? false : true;
}

#ifdef __cplusplus
}
#endif

#endif /* VMSL_H */
