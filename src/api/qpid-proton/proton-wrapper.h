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
#ifndef PROTON_WRAPPER_H
#define PROTON_WRAPPER_H

#include <math.h>
#include <sys/time.h>

#include <proton/message.h>
#include <proton/messenger.h>

#include <common/gru_portable.h>

#include "contrib/options.h"
#include "msgctxt.h"
#include "vmsl.h"
#include "proton-context.h"
#include "statistics.h"

#ifdef __cplusplus
extern "C" {
#endif

msg_ctxt_t *proton_init(stat_io_t *stat_io, void *data, gru_status_t *status);
void proton_stop(msg_ctxt_t *ctxt, gru_status_t *status);
void proton_destroy(msg_ctxt_t *ctxt, gru_status_t *status);

vmsl_stat_t proton_send(msg_ctxt_t *ctxt, msg_content_loader content_loader, gru_status_t *status);
vmsl_stat_t proton_subscribe(msg_ctxt_t *ctxt, void *data, gru_status_t *status);
vmsl_stat_t proton_receive(msg_ctxt_t *ctxt, msg_content_data_t *content, gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* PROTON_WRAPPER_H */
