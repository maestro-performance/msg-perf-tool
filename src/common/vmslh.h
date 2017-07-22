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

/**
 * VMSL callback signature
 */
typedef void (*vmslh_callback_fn)(void *ctxt, void *msg, void *payload);


/**
 * VMSL callback object
 */
typedef struct vmslh_callback_t_ {
  vmslh_callback_fn call;
  void *payload;
} vmslh_callback_t;

/**
 * A set of list of callbacks to be executed during different stages of messaging
 * processing
 */
typedef struct vmslh_handlers_t_ {
  gru_list_t *before_connect;
  gru_list_t *after_connect;
  gru_list_t *before_send;
  gru_list_t *after_send;
  gru_list_t *before_receive;
  gru_list_t *after_receive;
  gru_list_t *finalize_receive;
} vmslh_handlers_t;

/**
 * Add or replace a callback to the VMSL handlers list
 * @param list the list of VMSL handlers
 * @param callback the call back to add/replace
 * @param payload the payload to the callback
 * @param status status structure
 * @return true if successfully added/replaced or false otherwise
 */
bool vmslh_add(gru_list_t *list, vmslh_callback_fn callback, void *payload, gru_status_t *status);

/**
 * Removes a callback from the VMSL handlers list
 * @param list the list of VMSL handlers
 * @param callback the call back to remove
 */
void vmslh_remove(gru_list_t *list, vmslh_callback_fn callback);

/**
 * Runs a VMSL handlers list executing all the existent callbacks
 * @param list the list to add
 * @param ctxt the messaging context
 * @param msg the current message being processed (sent/received/etc) in VMSL
 */
void vmslh_run(gru_list_t *list, void *ctxt, void *msg);

/**
 * Create a new VMSL handlers structure
 * @param status status object
 * @return a new handlers structure or false otherwise (details available in the status object)
 */
vmslh_handlers_t vmslh_new(gru_status_t *status);

/**
 * Cleans up the VMSL handlers structure
 * @param handlers the structure to cleanup
 */
void vmslh_cleanup(vmslh_handlers_t *handlers);

#ifdef __cplusplus
}
#endif

#endif //MPT_VMSLH_H
