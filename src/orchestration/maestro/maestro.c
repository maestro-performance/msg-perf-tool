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
#include "maestro.h"

static bool cancel = false;
static pthread_t player;

static void *maestro_run(void *en) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Player is running");
	while (!cancel) {

		sleep(1);
	}

	logger(DEBUG, "Maestro is terminating");
	return NULL;
}

void maestro_init() {
	int sys_ret = pthread_create(&m, NULL, maestro_run, entry);
	if (sys_ret != 0) {
		logger(ERROR, "Unable to create probe thread");

		return;
	}
}

void