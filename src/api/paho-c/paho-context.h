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
#ifndef PAHO_CONTEXT_H
#define PAHO_CONTEXT_H

#include <log/gru_logger.h>
#include <network/gru_uri.h>

#include <MQTTClient.h>

#include "vmslh.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct paho_ctxt_t_ {
	MQTTClient client;
	gru_uri_t uri;
  vmslh_handlers_t *handlers;
} paho_ctxt_t;

paho_ctxt_t *paho_context_init(vmslh_handlers_t *handlers);
void paho_context_destroy(paho_ctxt_t **ctxt);

#ifdef __cplusplus
}
#endif

#endif /* PAHO_CONTEXT_H */
