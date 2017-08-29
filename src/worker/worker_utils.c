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

worker_queue_opt_t worker_queue_new_opt(worker_flags_t worker_flags) {
	worker_queue_opt_t queue_opt = {0};

	if (worker_flags & WRK_RECEIVER) {
		queue_opt.msg_size = sizeof(worker_snapshot_t);
		queue_opt.proj_id = 0x32;
	}
	else {
		queue_opt.msg_size = sizeof(worker_snapshot_t);
		queue_opt.proj_id = 0x33;
	}

	return queue_opt;
}


worker_queue_t *worker_create_queue(const worker_t *worker, gru_status_t *status) {
	const char *cname = worker_name(worker, getpid(), status);

	if (!cname) {
		return NULL;
	}

	worker_queue_opt_t queue_opt = worker_queue_new_opt(worker->worker_flags);
	worker_queue_t *queue = worker_queue_new(cname, PQUEUE_WRITE, queue_opt, status);
	gru_dealloc_const_string(&cname);

	if (!queue) {
		gru_status_set(status, GRU_FAILURE, "Unable to open a posix queue: %s",
					   status->message);

		return NULL;
	}

	return queue;
}


static int32_t worker_log_fill_last_dir(char *path, size_t max_len, gru_status_t *status) {
	int32_t ret = 0;

	do {

		snprintf(path, max_len, "%s/%"PRId32"", options_get_log_dir(), ret);

		bool exists = gru_path_exists(path, status);
		if (!exists) {
			return ret;
		}

		if (gru_status_error(status)) {
			return -1;
		}

		ret++;
	} while (ret < INT_MAX);

	return -1;
}

bool worker_log_init(char *worker_log_dir, gru_status_t *status) {
	int32_t last_log_dir = worker_log_fill_last_dir(worker_log_dir, PATH_MAX - 1, status);
	if (last_log_dir < 0) {
		return false;
	}

	if (!gru_path_mkdirs(worker_log_dir, status)) {
		return false;
	}

	return true;
}


void worker_log_link_create(const char *target, const char *basedir, const char *name) {
	char link_path[PATH_MAX] = {0};

	snprintf(link_path, sizeof(link_path) - 1, "%s/%s", basedir, name);

	unlink(link_path);
	symlink(target, link_path);
}