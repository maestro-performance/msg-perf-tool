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

typedef struct maestro_cmd_ctxt_t_ {
	vmsl_t vmsl;
	msg_ctxt_t *msg_ctxt;
} maestro_cmd_ctxt_t;

maestro_cmd_ctxt_t *maestro_cmd_ctxt_init(const gru_uri_t *uri, gru_status_t *status);
void maestro_cmd_ctxt_destroy(maestro_cmd_ctxt_t **ptr);

#endif /* MAESTRO_CMD_CTXT_H */
