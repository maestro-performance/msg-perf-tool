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

#ifndef MPT_VMSLH_H
#define MPT_VMSLH_H

#include <common/gru_status.h>
#include <common/gru_alloc.h>
#include <collection/gru_list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*vmslh_callback_fn)(void *ctxt, void *msg, void *payload);

typedef struct vmslh_callback_t_ {
  vmslh_callback_fn call;
  void *payload;
} vmslh_callback_t;

typedef struct vmslh_handlers_t_ {
  gru_list_t *before_connect;
  gru_list_t *after_connect;
  gru_list_t *before_send;
  gru_list_t *after_send;
  gru_list_t *before_receive;
  gru_list_t *after_receive;
  gru_list_t *finalize_receive;
} vmslh_handlers_t;


bool vmslh_add(gru_list_t *list, vmslh_callback_fn callback, void *payload, gru_status_t *status);
void vmslh_run(gru_list_t *list, void *ctxt, void *msg);

vmslh_handlers_t vmslh_new(gru_status_t *status);
void vmslh_cleanup(vmslh_handlers_t *handlers);

#ifdef __cplusplus
}
#endif

#endif //MPT_VMSLH_H
