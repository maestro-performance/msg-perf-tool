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
#include "process_utils.h"

static bool interrupted = false;

static char *remap_get_name(const char *base_name, pid_t parent, pid_t pid) {
	char *name;
	int ret = 0;

	if (parent == 0) {
		ret = asprintf(&name, "%s-%d.log", base_name, pid);
	} else {
		ret = asprintf(&name, "%s-%d-%d.log", base_name, parent, pid);
	}

	if (ret == -1) {
		return NULL;
	}

	return name;
}

bool remap_log(const char *dir,
	const char *base_name,
	pid_t parent,
	pid_t pid,
	FILE *fd,
	gru_status_t *status) {

	char *name = remap_get_name(base_name, parent, pid);
	if (!name) {
		gru_status_set(status, GRU_FAILURE, "Not enough memory to format the name");

		return false;
	}

	bool ret = gru_io_remap(dir, name, fd, status);
	free(name);
	return ret;
}

bool remap_log_with_link(const char *dir,
			   const char *base_name,
			   pid_t parent,
			   pid_t pid,
			   FILE *fd,
			   gru_status_t *status) {

	char *name = remap_get_name(base_name, parent, pid);
	if (!name) {
		gru_status_set(status, GRU_FAILURE, "Not enough memory to format the name");

		return false;
	}

	if (gru_io_remap(dir, name, fd, status)) {
		char link_path[PATH_MAX] = {0};

		snprintf(link_path, sizeof(link_path) - 1, "%s/%s-current.log", dir, base_name);

		char link_target[PATH_MAX] = {0};
		snprintf(link_target, sizeof(link_target) - 1, "%s/%s", dir, name);

		unlink(link_path);
		symlink(link_target, link_path);

		free(name);
		return true;
	}

	free(name);
	return false;
}


/**
 * Initializes the controller process if in daemon mode
 * @param background (ie: daemon mode ...)
 * @param logdir
 * @param controller_name
 * @return The controller PID if the parent, 0 if the child of the controller of -1 in
 * case of error.
 */
int init_controller(const char *logdir, const char *controller_name) {
	int controller = fork();

	if (controller == 0) {
		setsid();

		int fd = open("/dev/null", O_RDWR, 0);

		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);

			if (fd > 2) {
				close(fd);
			}
		}

		gru_status_t status = {0};

		bool ret = remap_log_with_link(logdir, controller_name, 0, getpid(), stderr, &status);

		if (!ret) {
			fprintf(
				stderr, "Unable to initialize the controller: %s", status.message);
			return -1;
		}

		return 0;
	} else if (controller > 0) {
		printf("%d\n", controller);
		return controller;
	} else {
		fprintf(stderr, "Unable to create child process");
		return -1;
	}
}

static void interrupt_handler(int signum) {
	interrupted = true;
}


void install_interrupt_handler() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &interrupt_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

inline bool is_interrupted() {
	return interrupted;
}

int create_queue(key_t key, gru_status_t *status) {
	int msg_flag = IPC_CREAT | 0666;

	int queue_id = msgget(key, msg_flag);
	if (queue_id < 0) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to create a POSIX queue: %s", strerror(errno));

		return -1;
	}

	return queue_id;
}