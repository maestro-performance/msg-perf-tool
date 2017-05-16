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
#include <limits.h>
#include <semaphore.h>
#include <signal.h>

#include <sys/wait.h>

#include <common/gru_status.h>

#include <ipc/shared_data_buffer.h>

static bool locked = true;

typedef struct tst_t_ { int value; } tst_t;

static void abstract_worker_sigusr2_handler(int signum) {
	locked = false;
}

void abstract_worker_setup_wait() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &abstract_worker_sigusr2_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR2, &sa, NULL);
	locked = true;
}

int main(int argc, char **argv) {
	volatile shr_data_buff_t *shr = NULL;
	gru_logger_set(gru_logger_default_printer);
	gru_status_t status = gru_status_new();
	const int expected = 29;
	int i = 0;

	pid_t child = fork();
	if (child == 0) {
		shr = shr_buff_new(BUFF_WRITE, sizeof(tst_t), "buildtest", &status);
		if (!shr) {
			fprintf(stderr, "Unable to open a write buffer: %s\n", status.message);

			return EXIT_FAILURE;
		}

		kill(getppid(), SIGUSR2);
		for (i = 0; i < 1000000; i++) {
			tst_t data = {0};
			data.value = i;

			if (!shr_buff_write(shr, &data, sizeof(tst_t))) {
				printf("Unable to write shared data: %d ...\n", i);
				usleep(100);
			}
		}

		shr_buff_detroy(&shr);
	} else {
		abstract_worker_setup_wait();
		while (locked) {
			int wstatus = 0;
			pid_t wret = waitpid(child, &wstatus, WNOHANG);

			if (wret != 0) {
				if (WIFEXITED(wstatus)) {
					fprintf(stderr,
						"Child %d finished with status %d\n",
						child,
						WEXITSTATUS(wstatus));

					return EXIT_FAILURE;
				} else if (WIFSIGNALED(wstatus)) {
					fprintf(stderr,
						"Child %d received a signal %d\n",
						child,
						WTERMSIG(wstatus));

					return EXIT_FAILURE;
				} else if (WIFSTOPPED(wstatus)) {
					fprintf(stderr, "Child %d stopped %d", child, WSTOPSIG(wstatus));

					return EXIT_FAILURE;
				}
			}

			usleep(50);
		}

		shr = shr_buff_new(BUFF_READ, sizeof(tst_t), "buildtest", &status);

		if (!shr) {
			fprintf(stderr, "Unable to open a read buffer: %s\n", status.message);

			kill(child, SIGTERM);
			return EXIT_FAILURE;
		}

		tst_t data = {0};
		for (i = 0; i < 100000; i++) {
			if (!shr_buff_read(shr, &data, sizeof(tst_t))) {
				usleep(100);
			}
			// printf("Read shared data : %d ...\n", data.value);

			if (data.value >= expected) {
				fprintf(stderr, "Found expected value %d\n", expected);

				break;
			}
		}

		if (data.value >= expected) {
			fprintf(stderr,
				"The read value %d is greater than or equal the expected value %d\n",
				data.value,
				expected);

			shr_buff_detroy(&shr);
			kill(child, SIGTERM);
			return EXIT_SUCCESS;
		}
		fprintf(stderr, "Finished counting and did not read data from the child\n");
		fprintf(stderr, "Last read value %d (expected value %d)\n", data.value, expected);

		shr_buff_detroy(&shr);
	}

	if (child == 0) {
		fprintf(stderr, "Finished counting and the parent did not find the value\n");
	} else {
		fprintf(stderr, "Child finished with errors (i == %d)?\n", i);
		kill(child, SIGTERM);
	}

	return EXIT_FAILURE;
}