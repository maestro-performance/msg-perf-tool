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
#include "message_sender.h"

void sender_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	stat_io_t *stat_io = statistics_init(SENDER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);

		return;
	}
	logger(TRACE, "Initializing test execution");

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER,
		.qos = MSG_QOS_AT_MOST_ONCE,
	};

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, opt, NULL, &status);
	if (!msg_ctxt) {
		fprintf(stderr, "%s", status.message);

		return;
	}

	install_timer(1);
	install_interrupt_handler();
	load_message_data(options, &status);

	mpt_timestamp_t last;
	mpt_timestamp_t start = statistics_now();

	register uint64_t sent = 0;
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

		last = statistics_now();

		if (last_calc <= (last.tv_sec - 10)) {
			statistics_throughput_partial(stat_io, start, last, sent);

			last_calc = last.tv_sec;
		}

		if (options->throttle > 0) {
			round++;
			if (round == options->throttle) {
				usleep(1000000 - last.tv_usec);
			}
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	unload_message_data();

	statistics_destroy(&stat_io);

	uint64_t elapsed = statistics_diff(start, last);
	double rate = ((double) sent / elapsed) * 1000;

	logger(STAT, "summary;sent;%" PRIu64 ";elapsed;%" PRIu64 ";rate;%.2f", sent, elapsed,
		rate);

	logger(INFO, "Summary: sent %" PRIu64 " messages in %" PRIu64
				 " milliseconds (rate: %.2f msgs/sec)",
		sent, elapsed, rate);
}