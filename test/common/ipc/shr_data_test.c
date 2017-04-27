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
#include <semaphore.h>
#include <signal.h>
#include <limits.h>

#include <common/gru_status.h>

#include <ipc/shared_data_buffer.h>

typedef struct tst_t_ {
	int value;
} tst_t;


int main(int argc, char **argv) {
	gru_logger_set(gru_logger_default_printer);
	gru_status_t status = gru_status_new();
	const int expected = 29;
	const int count = 45;

	pid_t child = fork();
	if (child == 0) {
		shr_data_buff_t *shr = shr_buff_new(BUFF_WRITE, sizeof(tst_t), "buildtest", &status);
		if (!shr) {
			fprintf(stderr, "Unable to open a write buffer: %s", status.message);
			
			return EXIT_FAILURE;
		}

		for (int i = 0; i < INT_MAX; i++) {
			tst_t data = {0};
			data.value = i;

			// printf("Writing shared data: %d ...\n", i);

			shr_buff_write(shr, &data, sizeof(tst_t));
		}
		shr_buff_detroy(&shr);
	}
	else {
		sleep(1);
		shr_data_buff_t *shr = shr_buff_new(BUFF_READ, sizeof(tst_t), "buildtest", &status);

		if (!shr) {
			fprintf(stderr, "Unable to open a read buffer: %s", status.message);
			
			kill(child, SIGTERM);
			return EXIT_FAILURE;
		}

		tst_t data = {0};
		for (int i = 0; i < INT_MAX; i++) {
			shr_buff_read(shr, &data, sizeof(tst_t));
			printf("Read shared data : %d ...\n", data.value);
			
			if (data.value >= expected) {
				fprintf(stderr, "Found expected value %d\n", expected);

				break;
			}
		}
		
		if (data.value >= expected) {
			fprintf(stderr, "The read value %d is greater than or equal the expected value %d\n", 
				data.value, expected);

			shr_buff_detroy(&shr);
			kill(child, SIGTERM);
			return EXIT_SUCCESS;
		}
		
		shr_buff_detroy(&shr);	
	}

	
	kill(child, SIGTERM);
	return EXIT_FAILURE;
}