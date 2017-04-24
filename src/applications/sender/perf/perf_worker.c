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

static bool perf_worker_init_data(msg_content_data_t *data, size_t size, gru_status_t *status) {
	msg_content_data_init(data, size, status);
	if (!gru_status_success(status)) {
		msg_content_data_release(data);

		return false;
	}

	msg_content_data_fill(data, 'd');
	return true;
}

static void perf_csv_name(const char *prefix, char *name, size_t len) 
{
	snprintf(name, len - 1, "%s-%d.csv.gz", prefix, getpid());
}

static bool perf_initialize_writer(stats_writer_t *writer, const options_t *options, 
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

void perf_worker_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	const uint32_t sample_interval = 10; // sampling interval
	uint64_t last_count = 0;

	logger(TRACE, "Initializing test execution");

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER, 
		.qos = MSG_QOS_AT_MOST_ONCE,
		.conn_info.id = MSG_CONN_ID_DEFAULT_SENDER_ID,
		.uri = options->uri
	};



	msg_ctxt_t *msg_ctxt = vmsl->init(opt, NULL, &status);
	if (!msg_ctxt) {
		fprintf(stderr, "%s", status.message);

		return;
	}

	// Initialize data to be sent
	msg_content_data_t content_storage = {0};
	if (!perf_worker_init_data(&content_storage, options->message_size, &status)) {
		goto err_exit;
	}

	// statistics_throughput_header(stat_io);
	stats_writer_t writer = {0};
	if (!perf_initialize_writer(&writer, options, &status)) {
		logger(ERROR, "Unable to initialize writer: %s", status.message);
		
		goto err_exit;
	}
	

	gru_timestamp_t start = gru_time_now();
	gru_timestamp_t now = start;
	gru_timestamp_t last_sample_ts = start; // Last sampling timestamp


	install_timer(1);
	install_interrupt_handler();

	register uint64_t count = 0;
	
	stat_throughput_t tp_out = {0};

	useconds_t idle_usec = 0;
	if (options->throttle) {
		idle_usec = 1000000 / options->throttle;
	}

	while (can_continue(options, count)) {
		vmsl_stat_t ret = vmsl->send(msg_ctxt, &content_storage, &status);
		if (vmsl_stat_error(ret)) {
			fprintf(stderr, "Unable to send message: %s\n", status.message);

			break;
		}

		count++;

		now = gru_time_now();

		if (gru_time_elapsed_secs(last_sample_ts, now) >= sample_interval) {
			uint64_t processed_count = count - last_count;

			calc_throughput(&tp_out, last_sample_ts, now, processed_count);

			if (unlikely(!writer.throughput.write(&tp_out, &status))) {
				logger(ERROR, "Unable to write throughput data: %s", status.message);

				gru_status_reset(&status);
				break;
			} 

			last_count = count;
			last_sample_ts = now;
		}

		if (options->throttle > 0) {
			usleep(idle_usec);
		}
	}

	msg_content_data_release(&content_storage);

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	writer.throughput.finalize(&status);

	uint64_t elapsed = gru_time_elapsed_secs(start, now);
	calc_throughput(&tp_out, start, now, count);

	logger(INFO,
		"Summary: sent %" PRIu64 " messages in %" PRIu64
		" milliseconds (rate: %.2f msgs/sec)",
		tp_out.count, elapsed, tp_out.rate);

	return;

	err_exit:
	fprintf(stderr, "%s", status.message);
	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		vmsl->destroy(msg_ctxt, &status);
	}

	gru_status_reset(&status);
	return;
}