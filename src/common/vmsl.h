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

#ifdef __cplusplus
extern "C" {
#endif

#include "msgctxt.h"
#include "statistics.h"

#include <log/gru_logger.h>

typedef msg_ctxt_t *(*msg_init)(stat_io_t *stat_io, void *data);
typedef void (*msg_send)(msg_ctxt_t *ctxt, msg_content_loader content_loader);
typedef void (*msg_subscribe)(msg_ctxt_t *ctxt, void *data);
typedef void (*msg_receive)(msg_ctxt_t *ctxt, msg_content_data_t *content);
typedef void (*msg_stop)(msg_ctxt_t *ctxt);
typedef void (*msg_commit)(msg_ctxt_t *ctxt, void *data);
typedef void (*msg_destroy)(msg_ctxt_t *);

typedef struct vmsl_t_ {
	msg_init init;
	msg_send send;
	msg_subscribe subscribe;
	msg_receive receive;
	msg_commit commit;
	msg_stop stop;
	msg_destroy destroy;
} vmsl_t;

vmsl_t *vmsl_init();
void vmsl_destroy(vmsl_t **vmsl);

#ifdef __cplusplus
}
#endif

#endif /* VMSL_H */
