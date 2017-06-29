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

#ifndef MPT_PAHO_HANDLERS_H
#define MPT_PAHO_HANDLERS_H

#include <common/gru_keypair.h>
#include <common/gru_variant.h>
#include <log/gru_logger.h>
#include <collection/gru_list.h>
#include <string/gru_util.h>

#include <MQTTClient.h>

#include "msgctxt.h"
#include "vmslh.h"
#include "mpt-debug.h"
#include "paho-context.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set default handlers for the protocol
 * @param handlers
 * @param opt
 * @param status
 */
void paho_set_default_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status);

/**
 * Set the user-defined handlers for the protocol
 * @param handlers
 * @param opt
 * @param status
 */
void paho_set_user_parameters(vmslh_handlers_t *handlers, msg_opt_t opt, gru_status_t *status);


void paho_set_keep_alive_interval(void *ctxt, void *conn_opts, void *payload);
void paho_set_clean_session(void *ctxt, void *conn_opts, void *payload);
void paho_set_qos_mode(void *ctxt, void *msg, void *payload);
void paho_set_retained(void *ctxt, void *msg, void *payload);

#ifdef __cplusplus
}
#endif


#endif //MPT_PAHO_HANDLERS_H
