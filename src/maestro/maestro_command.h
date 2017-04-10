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
#ifndef MAESTRO_COMMAND_H
#define MAESTRO_COMMAND_H

#include "vmsl.h"
#include "msgctxt.h"
#include "vmsl_assign.h"

typedef struct maestro_cmd_ctxt_t_ {
	vmsl_t vmsl;
	msg_ctxt_t *msg_ctxt;
} maestro_cmd_ctxt_t;

int maestro_cmd_start_receiver();


#endif /* MAESTRO_COMMAND_H */
