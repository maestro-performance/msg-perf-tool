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
#include "perf_worker.h"



static void perf_csv_name(const char *prefix, char *name, size_t len) 
{
	snprintf(name, len - 1, "%s-%d.csv.gz", prefix, getpid());
}

static bool perf_initialize_csv_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	csv_writer_throughput_assign(&writer->throughput);

	char tp_fname[64] = {0};
	perf_csv_name("sender-throughput", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) options->logdir;

	if (!writer->throughput.initialize(&tp_io_info, status)) {
		return false;
	}
	
	return true;
}

static bool perf_initialize_out_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	out_writer_latency_assign(&writer->latency);
	out_writer_throughput_assign(&writer->throughput);

	return true;
}

static bool perf_initialize_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	if (options->logdir) {
		return perf_initialize_csv_writer(writer, options, status);
	}
	
	return perf_initialize_out_writer(writer, options, status);
}

void perf_worker_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	
	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker_options_t wrk_opt = {0};
	worker.options = &wrk_opt;

	worker.options->uri = options->uri; 
	if (options->count == 0) {
		worker.options->duration_type = TEST_TIME;
		worker.options->duration.time = options->duration;
	}
	worker.options->parallel_count = options->parallel_count;
	worker.options->log_level = options->log_level;
	worker.options->message_size = options->message_size;
	worker.options->throttle = options->throttle;

	stats_writer_t writer = {0};
	worker.writer = &writer;
	perf_initialize_writer(worker.writer, options, &status);

	worker.can_continue = worker_check;
	

	worker_ret_t ret = {0}; 
	worker_snapshot_t snapshot = {0};

	ret = abstract_sender_worker_start(&worker, &snapshot, &status);
	if (ret != WORKER_SUCCESS) {
		fprintf(stderr, "Unable to execute worker: %s\n", status.message);

		return;
	}

	uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

	logger(INFO, 
	 	"Summary: sent %" PRIu64 " messages in %" PRIu64
	 	" seconds (rate: %.2f msgs/sec)",
	 	snapshot.count, elapsed, snapshot.throughput.rate);
	
	return;
}