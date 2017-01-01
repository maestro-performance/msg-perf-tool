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

#ifndef PAHO_WRAPPER_H
#define PAHO_WRAPPER_H

#include <network/gru_uri.h>

#include "contrib/options.h"
#include "msgctxt.h"
#include "paho-context.h"
#include "statistics.h"



#ifdef __cplusplus
extern "C" {
#endif

msg_ctxt_t *paho_init(stat_io_t *stat_io, void *data);
void paho_stop(msg_ctxt_t *ctxt);
void paho_destroy(msg_ctxt_t *ctxt);

void paho_send(msg_ctxt_t *ctxt, msg_content_loader content_loader);
void paho_subscribe(msg_ctxt_t *ctxt, void *data);
void paho_receive(msg_ctxt_t *ctxt, msg_content_data_t *content);

#ifdef __cplusplus
}
#endif

#endif /* PAHO_WRAPPER_H */

