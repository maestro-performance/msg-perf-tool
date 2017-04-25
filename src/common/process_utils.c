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

bool remap_log(const char *dir, const char *base_name, pid_t parent, pid_t pid, FILE *fd,
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

/**
 * Initializes the controller process if in daemon mode
 * @param background (ie: daemon mode ...)
 * @param logdir
 * @param controller_name
 */
void init_controller(bool background, const char *logdir, const char *controller_name) {
	if (background) {
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

			bool ret = remap_log(logdir, controller_name, 0, getpid(), stderr, &status);

			if (!ret) {
				fprintf(
					stderr, "Unable to initialize the controller: %s", status.message);
				exit(1);
			}
		} else {
			if (controller > 0) {
				printf("%d\n", controller);
				exit(0);
			} else {
				fprintf(stderr, "Unable to create child process");
				exit(1);
			}
		}
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
}

inline bool is_interrupted() {
	return interrupted;
}

bool can_continue(const options_t *options, uint64_t sent) {
	if (is_interrupted()) {
		return false;
	}

	if (likely(options->count == 0)) {
		struct timeval now;

		gettimeofday(&now, NULL);

		if (likely(now.tv_sec <= options->duration.end.tv_sec)) {
			return true;
		}
	} else {
		if (likely(sent < options->count)) {
			return true;
		}
	}

	return false;
}


int create_queue(key_t key, gru_status_t *status) {
	int msg_flag = IPC_CREAT | 0666;
	
	int queue_id = msgget(key, msg_flag);
	if (queue_id < 0) {
		gru_status_set(status, GRU_FAILURE, 
			"Unable to create a POSIX queue: %s", strerror(errno));

		return -1;
	}

	return queue_id;
}