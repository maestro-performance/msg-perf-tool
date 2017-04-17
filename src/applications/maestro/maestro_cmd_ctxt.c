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
 #include "maestro_cmd_ctxt.h"

maestro_cmd_ctxt_t *maestro_cmd_ctxt_init(const gru_uri_t *uri, gru_status_t *status) {
	maestro_cmd_ctxt_t *ret = gru_alloc(sizeof(maestro_cmd_ctxt_t), status);
	gru_alloc_check(ret, NULL);

	ret->vmsl = vmsl_init();

	if (!vmsl_assign_by_url(uri, &ret->vmsl)) {
		return NULL;
	}

	return ret;
}

void maestro_cmd_ctxt_destroy(maestro_cmd_ctxt_t **ptr) {
	maestro_cmd_ctxt_t *ctxt = *ptr;

	if (!ctxt) {
		return;
	}

	gru_status_t tmp = gru_status_new();
	ctxt->vmsl.stop(ctxt->msg_ctxt, &tmp);

	gru_dealloc((void **) ptr);
}
