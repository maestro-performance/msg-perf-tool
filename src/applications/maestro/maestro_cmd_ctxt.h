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

#ifndef MAESTRO_CMD_CTXT_H
#define MAESTRO_CMD_CTXT_H

#include <common/gru_status.h>

#include "contrib/options.h"
#include "msgctxt.h"
#include "vmsl.h"
#include "vmsl_assign.h"

#include "maestro_forward_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct maestro_cmd_ctxt_t_ {
	vmsl_t vmsl;
  	vmslh_handlers_t handlers;
	msg_ctxt_t *msg_ctxt;
	int queue;
} maestro_cmd_ctxt_t;

/**
 * @brief A command context abstracts the command exchange via VMSL
 */

/**
 * Creates new command context
 * @param uri
 * @param status
 * @return
 */
maestro_cmd_ctxt_t *maestro_cmd_ctxt_new(const gru_uri_t *uri, gru_status_t *status);

/**
 * Destroys a command context
 * @param ptr
 */
void maestro_cmd_ctxt_destroy(maestro_cmd_ctxt_t **ptr);

/**
 * Starts the command context
 * @param cmd_ctxt
 * @param opt
 * @param status
 * @return
 */
bool maestro_cmd_ctxt_start(maestro_cmd_ctxt_t *cmd_ctxt, msg_opt_t opt, gru_status_t *status);

/**
 * Stops the command context
 * @param cmd_ctxt
 * @param status
 * @return
 */
void maestro_cmd_ctxt_stop(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);

/**
 * Subscribe to a maestro topic for command forwarding
 * @param cmd_ctxt command context
 * @param mtopic multiple topic specification
 * @param status status
 * @return
 */
bool maestro_cmd_ctxt_forwarder(maestro_cmd_ctxt_t *cmd_ctxt, vmsl_mtopic_spec_t *mtopic,
								gru_status_t *status);

/**
 * Send a maestro command via VMSL
 * @param cmd_ctxt
 * @param cmd
 * @param status
 * @return
 */
bool maestro_cmd_ctxt_send(maestro_cmd_ctxt_t *cmd_ctxt, msg_content_data_t *cmd, gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* MAESTRO_CMD_CTXT_H */
