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
#ifndef STOMP_CONTEXT_H
#define STOMP_CONTEXT_H

#include <litestomp-0/stomp_messenger.h>

#include <log/gru_logger.h>

#include "vmslh.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stomp_ctxt_t_ {
  	stomp_messenger_t *messenger;
	vmslh_handlers_t *handlers;
} stomp_ctxt_t;

stomp_ctxt_t *litestomp_context_init(vmslh_handlers_t *handlers);
void litestomp_context_destroy(stomp_ctxt_t **ctxt);

#ifdef __cplusplus
}
#endif

#endif /* STOMP_CONTEXT_H */
