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
#include "tune_worker.h"

typedef struct perf_stats_t_ { uint64_t sent; } perf_stats_t;

static bool tune_worker_init_data(msg_content_data_t *data, size_t size, gru_status_t *status) {
	msg_content_data_init(data, size, status);
	if (!gru_status_success(status)) {
		msg_content_data_release(data);

		return false;
	}

	msg_content_data_fill(data, 'e');
	return true;
}


static void tune_print_stat(uint32_t steps, const char *msg, ...) {
	va_list ap;

	printf("%s%s[Step %d] %s", RESET, LIGHT_WHITE, steps, RESET);

	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);
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
	if (gru_status_error(status)) {
		fprintf(stderr, "Unable to purge queue\n");
	}

	ret = ctxt->api->queue_reset(ctxt->handle, cap, name, status);
	if (gru_status_error(status)) {
		fprintf(stderr, "Unable to reset queue counters\n");
	}

	return ret;
}

static bool tune_initialize_out_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	out_writer_latency_assign(&writer->latency);
	out_writer_throughput_assign(&writer->throughput);

	return true;
}


static bool tune_exec_step(const options_t *options, const vmsl_t *vmsl,
	const gru_duration_t duration, const uint32_t throttle, worker_snapshot_t *snapshot) 
{
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	
	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker_options_t wrk_opt = {0};
	worker.options = &wrk_opt;

	worker.options->uri = options->uri; 
	if (options->count == 0) {
		worker.options->duration_type = TEST_TIME;
		worker.options->duration.time = duration;
	}
	worker.options->parallel_count = options->parallel_count;
	worker.options->log_level = options->log_level;
	worker.options->message_size = options->message_size;
	worker.options->throttle = throttle;

	stats_writer_t writer = {0};
	worker.writer = &writer;
	tune_initialize_out_writer(worker.writer, options, &status);

	worker.can_continue = worker_check;
	
	worker_ret_t ret = {0}; 

	ret = abstract_sender_worker_start(&worker, snapshot, &status);
	if (ret != WORKER_SUCCESS) {
		fprintf(stderr, "Unable to execute worker: %s\n", status.message);

		return false;
	}

	uint64_t elapsed = gru_time_elapsed_secs(snapshot->start, snapshot->now);

	logger(INFO, 
	 	"Summary: received %" PRIu64 " messages in %" PRIu64
	 	" seconds (rate: %.2f msgs/sec)",
	 	snapshot->count, elapsed, snapshot->throughput.rate);
	
	return true;
}

uint32_t tune_calc_approximate(worker_snapshot_t snapshot, bmic_queue_stat_t qstat,
	gru_duration_t duration, gru_status_t *status) {

	uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

	double approximate = ((double) (snapshot.count - qstat.queue_size)) / (double) elapsed;

	return (uint32_t) trunc(approximate);
}


int tune_worker_start(const vmsl_t *vmsl, const options_t *options) {
	gru_status_t status = gru_status_new();
	logger_t logger = gru_logger_get();

	logger(INFO, "Initializing tune");
	uint32_t steps = 5;
	uint64_t duration[5] = {1, 2, 4, 8, 10};

	bmic_context_t ctxt = {0};
	bool ret_ctxt = mpt_init_bmic_ctxt(options, &ctxt, &status);
	if (!ret_ctxt) {
		fprintf(stderr, "%s\n", status.message);

		return EXIT_FAILURE;
	}

	uint16_t multiplier[5] = {2, 3, 5, 10, 20};

	uint32_t approximate = 0;
	for (int i = 0; i < steps; i++) {
		printf(CLEAR_LINE);
		tune_print_stat(i, "Cleaning the queue");
		bool tret = tune_purge_queue(&ctxt, options, &options->uri.path[1], &status);
		if (!tret) {
			bmic_context_cleanup(&ctxt);
			return EXIT_FAILURE;
		}

		printf(CLEAR_LINE);
		gru_duration_t duration_object = gru_duration_from_minutes(duration[i]);
		tune_print_stat(i, "Duration %" PRIu64 " minutes\n", duration[i]);

		
		worker_snapshot_t snapshot = {0};

		if (!tune_exec_step(options, vmsl, duration_object, approximate, &snapshot)) {
			fprintf(stderr, "Step %d did not finish successfully", i);
		}
		else { 
			tune_print_stat(i, "Step %d finished sending data. Reading queue stats\n", i);
		}

		bmic_queue_stat_t qstats = {0};
		mpt_get_queue_stats(&ctxt, &options->uri.path[1], &qstats, &status);
		if (gru_status_error(&status)) {
			fprintf(stderr, "Error: %s\n", status.message);

			bmic_context_cleanup(&ctxt);
			return EXIT_FAILURE;
		}
		printf("Queue size: %" PRId64 "\n", qstats.queue_size);

		tune_print_stat(i, "Calculating approximate sustained throughput");
		approximate = tune_calc_approximate(snapshot, qstats, duration_object, &status);

		printf(CLEAR_LINE);
		tune_print_stat(i,
			"Approximate sustained throughput before applying multiplier: %" PRIu32 "\n",
			approximate);

		approximate += (approximate / multiplier[i]);

		tune_print_stat(i,
			"Sent: %" PRIu64 ". Queue size. %" PRId64 ". Received %" PRIu64
			". Approximate: %" PRIu32 "\n",
			snapshot.count,
			qstats.queue_size,
			qstats.msg_ack_count,
			approximate);
		tune_print_stat(i, "Sleeping for 10 seconds to let the receiver catch up\n");
		sleep(10);
	}

	bmic_context_cleanup(&ctxt);
	return EXIT_SUCCESS;
}