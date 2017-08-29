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
#include "stomp-context.h"

stomp_ctxt_t *litestomp_context_init(vmslh_handlers_t *handlers, gru_status_t *status) {
	stomp_ctxt_t *ret = gru_alloc(sizeof(stomp_ctxt_t), status);
	gru_alloc_check(ret, NULL);

	ret->handlers = handlers;

	return ret;
}

void litestomp_context_destroy(stomp_ctxt_t **ctxt) {
	free(*ctxt);
	*ctxt = NULL;
}