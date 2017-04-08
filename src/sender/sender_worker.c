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
#include "sender_worker.h"

void sender_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	const uint32_t tp_interval = 10;
	uint64_t last_count = 0;

	stat_io_t *stat_io = statistics_init(SENDER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);

		return;
	}
	logger(TRACE, "Initializing test execution");

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER, .qos = MSG_QOS_AT_MOST_ONCE,
	};

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, opt, NULL, &status);
	if (!msg_ctxt) {
		fprintf(stderr, "%s", status.message);

		return;
	}

	install_timer(1);
	install_interrupt_handler();
	load_message_data(options, &status);

	gru_timestamp_t last;
	gru_timestamp_t start = gru_time_now();

	register uint64_t sent = 0;

	/*
	 * Stores the number of sent messages if and only if throttle is enabled
	 */
	register uint64_t round = 0;
	time_t last_calc = 0;

	statistics_throughput_header(stat_io);
	while (can_continue(options, sent)) {
		vmsl_stat_t ret = vmsl->send(msg_ctxt, content_loader, &status);
		if (vmsl_stat_error(ret)) {
			fprintf(stderr, "Unable to send message: %s\n", status.message);

			break;
		}

		sent++;

		last = gru_time_now();

		if (last_calc <= (last.tv_sec - tp_interval)) {
			uint64_t processed_count = sent - last_count;

			statistics_throughput_partial(stat_io, last, tp_interval, processed_count);

			last_calc = last.tv_sec;
			last_count = sent;
		}

		if (options->throttle > 0) {
			round++;
			if (round == options->throttle) {
				usleep(1000000 - last.tv_usec);
				round = 0;
			}
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	unload_message_data();

	statistics_destroy(&stat_io);

	uint64_t elapsed = statistics_diff(start, last);
	double rate = ((double) sent / (double) elapsed) * 1000;

	logger(STAT,
		"summary;sent;%" PRIu64 ";elapsed;%" PRIu64 ";rate;%.2f",
		sent,
		elapsed,
		rate);

	logger(INFO,
		"Summary: sent %" PRIu64 " messages in %" PRIu64
		" milliseconds (rate: %.2f msgs/sec)",
		sent,
		elapsed,
		rate);
}