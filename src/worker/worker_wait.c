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
#include "worker_wait.h"

static bool locked = true;
static const int32_t wait_time = 50;

static int32_t limit;

static void abstract_worker_sigusr2_handler(int signum) {
	locked = false;
}

void worker_wait_setup() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &abstract_worker_sigusr2_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR2, &sa, NULL);
	locked = true;
	limit = 1000000 / wait_time;
}

bool worker_wait() {
	while (locked && limit > 0) {
		usleep(50);
		limit--;

		if (limit == 0) {
			locked = false;
			return false;
		}
	}

	return true;
}