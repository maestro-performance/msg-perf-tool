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
#include "tune_sender.h"

typedef struct perf_stats_t_ { uint64_t sent; } perf_stats_t;

static bool tune_can_continue(gru_duration_t duration) {
	struct timeval now;

	gettimeofday(&now, NULL);

	if (likely(now.tv_sec <= duration.end.tv_sec)) {
		return true;
	}

	return false;
}


static void tune_print_stat(uint32_t steps, const char *msg, ...) {
	va_list ap;

	printf("%s%s[Step %d] %s", RESET, LIGHT_WHITE, steps, RESET);

	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);
}

static bool tune_get_queue_stats(const bmic_context_t *ctxt, const options_t *options,
	const char *name, bmic_queue_stat_t *stat, gru_status_t *status) {
	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);

	if (!cap) {
		fprintf(stderr, "Unable to load capabilities\n");
		return false;
	}

	*stat = ctxt->api->queue_stats(ctxt->handle, cap, name, status);
	if (status->code != GRU_SUCCESS) {
		fprintf(stderr, "Unable to read queue stats\n");
		return false;
	}
	printf("Queue size: %" PRId64 "\n", stat->queue_size);

	return true;
}

static bool tune_purge_queue(const bmic_context_t *ctxt, const options_t *options,
	const char *name, gru_status_t *status) {

	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);
	if (!cap) {
		fprintf(stderr, "Unable to load capabilities\n");
		return false;
	}

	bool ret = false;

	ret = ctxt->api->queue_purge(ctxt->handle, cap, name, status);
	if (status->code != GRU_SUCCESS) {
		fprintf(stderr, "Unable to purge queue\n");
	}

	ret = ctxt->api->queue_reset(ctxt->handle, cap, name, status);
	if (status->code != GRU_SUCCESS) {
		fprintf(stderr, "Unable to reset queue counters\n");
	}

	return ret;
}

static perf_stats_t tune_exec_step(const options_t *options, const vmsl_t *vmsl,
	uint32_t step, gru_duration_t duration, uint32_t throttle) {
	perf_stats_t ret = {0};
	gru_status_t status = gru_status_new();

	// Open stdout ... never FAIL.
	stat_io_t *stat_io = statistics_init_stdout(SENDER, NULL);

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER,
		.qos = MSG_QOS_AT_MOST_ONCE,
	};

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, opt, NULL, &status);
	if (!msg_ctxt) {
		fprintf(stderr, "%s", status.message);

		return ret;
	}

	register uint64_t sent = 0;
	register uint64_t round = 0;


	while (tune_can_continue(duration)) {
		vmsl_stat_t sstat = vmsl->send(msg_ctxt, content_loader, &status);

		if (vmsl_stat_error(sstat)) {
			fprintf(stderr, "%s", status.message);
			break;
		}

		sent++;

		if (throttle > 0) {
			gru_timestamp_t now = gru_time_now();

			round++;

			if (round == throttle) {
				round = 0;
				usleep(1000000 - now.tv_usec);
			}
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	ret.sent = sent;
	return ret;
}

uint32_t tune_calc_approximate(perf_stats_t stats, bmic_queue_stat_t qstat,
	gru_duration_t duration, gru_status_t *status) {

	uint64_t elapsed = gru_duration_seconds(duration);

	double approximate = (stats.sent - qstat.queue_size) / elapsed;

	return trunc(approximate);
}

static bool tune_init_bmic_ctxt(
	const options_t *options, bmic_context_t *ctxt, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	gru_uri_t uri = gru_uri_parse(options->url, status);

	logger(INFO, "Resolved host to %s", uri.host);

	bool ret = bmic_context_init_simple(ctxt, uri.host, "admin", "admin", status);
	gru_uri_cleanup(&uri);

	if (!ret) {
		bmic_context_cleanup(ctxt);
		return false;
	}

	// Load the capabilities just so that it is cached
	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);
	if (!cap) {
		bmic_context_cleanup(ctxt);
		return false;
	}

	return true;
}



int tune_start(const vmsl_t *vmsl, const options_t *options) {
	gru_status_t status = gru_status_new();
	logger_t logger = gru_logger_get();

	logger(INFO, "Initializing tune");
	uint32_t steps = 5;
	uint64_t duration[5] = {1, 2, 4, 8, 10};

	bmic_context_t ctxt = {0};
	bool ret_ctxt = tune_init_bmic_ctxt(options, &ctxt, &status);
	if (!ret_ctxt) {
		fprintf(stderr, "%s\n", status.message);

		return EXIT_FAILURE;
	}

	uint16_t multiplier[5] = {2, 3, 5, 10, 20};

	uint32_t approximate = 0;
	for (int i = 0; i < steps; i++) {
		printf(CLEAR_LINE);
		tune_print_stat(i, "Cleaning the queue");
		bool tret = tune_purge_queue(&ctxt, options, "test.performance.queue", &status);
		if (!tret) {
			bmic_context_cleanup(&ctxt);
			return EXIT_FAILURE;
		}

		printf(CLEAR_LINE);
		gru_duration_t duration_object = gru_duration_from_minutes(duration[i]);
		tune_print_stat(i, "Duration %" PRIu64 " minutes\n", duration[i]);


		perf_stats_t pstats =
			tune_exec_step(options, vmsl, i, duration_object, approximate);
		tune_print_stat(i, "Step %d finished sending data. Reading queue stats\n", i);

		bmic_queue_stat_t qstats = {0};
		bool stat_ret = tune_get_queue_stats(&ctxt, options, "test.performance.queue",
											 &qstats, &status);
		if (!stat_ret) {
			fprintf(stderr, "Error: %s\n", status.message);

			bmic_context_cleanup(&ctxt);
			return EXIT_FAILURE;
		}


		tune_print_stat(i, "Calculating approximate sustained throughput");
		approximate = tune_calc_approximate(pstats, qstats, duration_object, &status);

		printf(CLEAR_LINE);
		tune_print_stat(i, "Approximate sustained throughput before applying multiplier: %" PRIu32
			   "\n",
			approximate);

		approximate += (approximate / multiplier[i]);

		tune_print_stat(i, "Sent: %" PRIu64 ". Queue size. %" PRId64 ". Received %" PRIu64
			   ". Approximate: %" PRIu32 "\n",
			pstats.sent, qstats.queue_size, qstats.msg_ack_count, approximate);
		tune_print_stat(i, "Sleeping for 10 seconds to let the receiver catch up\n");
		sleep(10);
	}

	bmic_context_cleanup(&ctxt);
	return EXIT_SUCCESS;
}