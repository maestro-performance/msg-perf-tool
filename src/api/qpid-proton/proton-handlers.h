/**
 Copyright 2017 Otavio Rodolfo Piske

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

#ifndef MPT_PROTON_HANDLERS_H
#define MPT_PROTON_HANDLERS_H

#include <proton/codec.h>
#include <proton/message.h>

#include <common/gru_keypair.h>
#include <common/gru_variant.h>
#include <log/gru_logger.h>
#include <collection/gru_list.h>
#include <string/gru_util.h>


#include "msgctxt.h"
#include "vmslh.h"
#include "mpt-debug.h"

void proton_param_cleanup();

void proton_set_default_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status);
void proton_set_user_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status);

void proton_set_properties(void *ctxt, void *msg, void *payload);
void proton_log_body_type(void *ctxt, void *msg, void *payload);
void proton_set_content_type(void *ctxt, void *msg, void *payload);
void proton_set_default_message_properties(void *ctxt, void *msg, void *payload);
void proton_set_ttl(void *ctxt, void *msg, void *payload);
void proton_set_durable(void *ctxt, void *msg, void *payload);
void proton_set_priority(void *ctxt, void *msg, void *payload);

#endif //MPT_PROTON_HANDLERS_H
