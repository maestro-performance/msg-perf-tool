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

#include <common/gru_status.h>
#include <statistics/latency_store.h>

static int simple_test() {
	gru_status_t status = gru_status_new();
	gru_logger_set(gru_logger_default_printer);

	if (!latency_store_init_count(25, 1, &status)) {
		fprintf(stderr, status.message);

		return EXIT_FAILURE;
	}

	latency_store_add(99);
	latency_store_add(99);
	latency_store_add(98);
	latency_store_add(96);
	latency_store_add(95);
	latency_store_add(93);
	latency_store_add(89);
	latency_store_add(72);
	latency_store_add(71);
	latency_store_add(70);
	latency_store_add(69);
	latency_store_add(69);
	latency_store_add(68);
	latency_store_add(66);
	latency_store_add(62);
	latency_store_add(61);
	latency_store_add(56);
	latency_store_add(54);
	latency_store_add(43);
	latency_store_add(88);
	latency_store_add(87);
	latency_store_add(85);
	latency_store_add(79);
	latency_store_add(78);
	latency_store_add(77);

	double ret = latency_store_calc_percentile(0.9);
	if ((int64_t) round(ret) != 98L) {
		fprintf(stderr, "Invalid value: %"PRId64"\n", (int64_t) round(ret));

		return EXIT_FAILURE;
	}

	ret = latency_store_calc_percentile(0.2);
	if ((int64_t) round(ret) != 64L) {
		fprintf(stderr, "Invalid value: %"PRId64"\n", (int64_t) round(ret));

		return EXIT_FAILURE;
	}

	latency_store_release();

	return EXIT_SUCCESS;
}

int long_test() {
	gru_status_t status = gru_status_new();
	gru_logger_set(gru_logger_default_printer);

	if (!latency_store_init_count(1200000, 1, &status)) {
		fprintf(stderr, status.message);

		return EXIT_FAILURE;
	}


	for (int32_t i = 0; i < 100000000; i++) {
		if (!latency_store_add(i)) {
			latency_store_release();
			fprintf(stderr, "Not enough memory to add record %"PRIi32"\n", i);

			return EXIT_SUCCESS;
		}
	}

	double ret = latency_store_calc_percentile(0.9);
	if ((int64_t) round(ret) != 89999999L) {
		fprintf(stderr, "Invalid value: %"PRId64"\n", (int64_t) round(ret));

		return EXIT_FAILURE;
	}

	latency_store_release();

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Missing test case name\n");

		return EXIT_FAILURE;
	}

	if (strncmp(argv[1], "simple", 2) == 0) {
		return simple_test();
	} else if (strcmp(argv[1], "long-test") == 0) {
		return long_test();
	}

	return EXIT_FAILURE;
}