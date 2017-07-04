/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "worker_utils.h"

void worker_msg_opt_setup(msg_opt_t *opt, msg_direction_t direction, const worker_options_t *options) {
	opt->direction = direction;
	opt->statistics = MSG_STAT_DEFAULT;

	msg_conn_info_gen_id(&opt->conn_info);
	opt->uri = options->uri;
}

void worker_msg_opt_cleanup(msg_opt_t *opt) {
	msg_conn_info_cleanup(&opt->conn_info);
}


bool worker_check(const worker_options_t *options, const worker_snapshot_t *snapshot) {
	if (is_interrupted()) {
		return false;
	}

	if (likely(options->duration_type == TEST_TIME)) {
		if (likely(snapshot->now.tv_sec <= options->duration.time.end.tv_sec)) {
			return true;
		}
	} else {
		if (likely(snapshot->count < options->duration.count)) {
			return true;
		}
	}

	return false;
}

const char *worker_name(const worker_t *worker, pid_t child, gru_status_t *status) {
	char *cname = NULL;

	if (asprintf(&cname, "%s-%d", worker->name, child) == -1) {
		gru_status_set(status, GRU_FAILURE, "Not enough memory to format name");

		return NULL;
	}

	return cname;
}

volatile shr_data_buff_t *worker_shared_buffer_new(const worker_t *worker,
	gru_status_t *status)
{
	const char *cname = worker_name(worker, getpid(), status);

	if (!cname) {
		return NULL;
	}

	volatile shr_data_buff_t *shr = shr_buff_new(BUFF_WRITE, sizeof(worker_snapshot_t),
		cname, status);
	gru_dealloc_const_string(&cname);

	if (!shr) {
		gru_status_set(status, GRU_FAILURE, "Unable to open a write buffer: %s",
			status->message);

		return NULL;
	}

	// If forked, then we need to signal the parent process that it's ok to continue
	if (worker->worker_flags & WRK_FORKED) {
		kill(getppid(), SIGUSR2);
	}

	return shr;
}
