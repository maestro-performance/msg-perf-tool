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

bool remap_log(const char *dir,
	const char *base_name,
	pid_t parent,
	pid_t pid,
	FILE *fd,
	gru_status_t *status) {
	char name[64];

	bzero(name, sizeof(name));

	if (parent == 0) {
		snprintf(name, sizeof(name) - 1, "%s-%d.log", base_name, pid);
	} else {
		snprintf(name, sizeof(name) - 1, "%s-%d-%d.log", base_name, parent, pid);
	}

	return gru_io_remap(dir, name, fd, status);
}

bool remap_log_with_link(const char *dir,
			   const char *base_name,
			   pid_t parent,
			   pid_t pid,
			   FILE *fd,
			   gru_status_t *status) {
	char name[64];

	bzero(name, sizeof(name));

	if (parent == 0) {
		snprintf(name, sizeof(name) - 1, "%s-%d.log", base_name, pid);
	} else {
		snprintf(name, sizeof(name) - 1, "%s-%d-%d.log", base_name, parent, pid);
	}

	if (gru_io_remap(dir, name, fd, status)) {
		char link_path[PATH_MAX] = {0};

		snprintf(link_path, sizeof(link_path) - 1, "%s/%s-current.log", dir, base_name);

		char link_target[PATH_MAX] = {0};
		snprintf(link_target, sizeof(link_target) - 1, "%s/%s", dir, name);

		unlink(link_path);
		symlink(link_target, link_path);

		return true;
	}


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

static void timer_handler(int signum) {
	// NO-OP for now
}

static void interrupt_handler(int signum) {
	interrupted = true;
}

void install_timer(time_t sec) {
	struct sigaction sa;
	struct itimerval timer;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &timer_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGALRM, &sa, NULL);

	timer.it_value.tv_sec = sec;
	timer.it_value.tv_usec = 0;

	timer.it_interval.tv_sec = sec;
	timer.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &timer, NULL);
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