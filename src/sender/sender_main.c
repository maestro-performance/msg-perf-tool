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
#include "sender_main.h"

void show_help() {
	gru_cli_general_usage("mpt-sender");

	gru_cli_program_description(
		"perf", "Runs a performance test against a message broker");
	gru_cli_program_description(
		"tune", "Tries to automatically determine the sustained throughput");
}

int main(int argc, char **argv) {
	if (argc < 2) {
		show_help();

		return EXIT_FAILURE;
	} else {
		if (strncmp(argv[1], "perf", 4) == 0) {
			return perf_main((argc - 1), &argv[1]);
		} else if (strncmp(argv[1], "tune", 4) == 0) {
			return tune_main((argc - 1), &argv[1]);
		}
	}

	return EXIT_SUCCESS;
}
