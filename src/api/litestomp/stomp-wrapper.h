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
#ifndef STOMP_WRAPPER_H
#define STOMP_WRAPPER_H

#include <log/gru_logger.h>
#include <time/gru_time_utils.h>

#include <litestomp-0/stomp_messenger.h>

#include "msgctxt.h"
#include "vmsl.h"
#include "vmslh.h"

#include "stomp-context.h"

#ifdef __cplusplus
extern "C" {
#endif

msg_ctxt_t *litestomp_init(msg_opt_t opt, vmslh_handlers_t *handlers, gru_status_t *status);
vmsl_stat_t litestomp_start(msg_ctxt_t *ctxt, gru_status_t *status);
void litestomp_stop(msg_ctxt_t *ctxt, gru_status_t *status);
void litestomp_destroy(msg_ctxt_t *ctxt, gru_status_t *status);

vmsl_stat_t
	litestomp_send(msg_ctxt_t *ctxt, msg_content_data_t *data, gru_status_t *status);
vmsl_stat_t litestomp_subscribe(msg_ctxt_t *ctxt, vmsl_mtopic_spec_t *mtopic, gru_status_t *status);
vmsl_stat_t litestomp_receive(msg_ctxt_t *ctxt,
	msg_content_data_t *content,
	gru_status_t *status);

vmsl_info_t litestomp_info();

bool litestomp_vmsl_assign(vmsl_t *vmsl);

#ifdef __cplusplus
}
#endif

#endif /* STOMP_WRAPPER_H */
