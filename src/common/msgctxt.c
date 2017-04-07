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
#include "msgctxt.h"

msg_ctxt_t *msg_ctxt_init(stat_io_t *stat_io, gru_status_t *status) {
	msg_ctxt_t *ret = gru_alloc(sizeof(msg_ctxt_t), status);
	gru_alloc_check(ret, NULL);

	ret->stat_io = stat_io;
	ret->msg_opts.qos = MSG_QOS_AT_MOST_ONCE;
	ret->msg_opts.statistics = MSG_STAT_DEFAULT;

	return ret;
}

void msg_ctxt_destroy(msg_ctxt_t **ctxt) {
	free(*ctxt);
	*ctxt = NULL;
}