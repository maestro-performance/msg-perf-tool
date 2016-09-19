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
#ifndef PROTON_CONTEXT_H
#define PROTON_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <log/gru_logger.h>
    
#include <proton/messenger.h>
    
typedef struct proton_ctxt_t_ {
    pn_messenger_t *messenger;
} proton_ctxt_t;

proton_ctxt_t *proton_context_init();
void proton_context_destroy(proton_ctxt_t **ctxt);


#ifdef __cplusplus
}
#endif

#endif /* PROTON_CONTEXT_H */

