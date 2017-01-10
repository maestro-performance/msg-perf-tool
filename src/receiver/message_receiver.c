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
#include "message_receiver.h"
#include "vmsl.h"

static bool interrupted = false;

static void timer_handler(int signum) {
	// NO-OP
}

static void interrupt_handler(int signum) { interrupted = true; }

static void install_timer() {
	struct sigaction sa;
	struct itimerval timer;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &timer_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGALRM, &sa, NULL);

	timer.it_value.tv_sec = 30;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 30;
	timer.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &timer, NULL);
}

static void install_interrupt_handler() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &interrupt_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &sa, NULL);
}

static bool can_continue(const options_t *options) {
	struct timeval now;

	if (interrupted) {
		return false;
	}

	if (options->duration.end.tv_sec == 0) {
		return true;
	}

	gettimeofday(&now, NULL);

	if (now.tv_sec >= options->duration.end.tv_sec) {
		return false;
	}

	return true;
}

void receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	stat_io_t *stat_io = statistics_init(RECEIVER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);

		return;
	}

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, NULL, &status);
	if (!msg_ctxt) {
		fprintf(stderr, "%s", status.message);

		return;
	}

	vmsl_stat_t ret = vmsl->subscribe(msg_ctxt, NULL, &status);
	if (vmsl_stat_error(ret)) {
		fprintf(stderr, "%s", status.message);

		statistics_destroy(&stat_io);
		vmsl->destroy(msg_ctxt, &status);
		return;
	}

	install_timer();
	install_interrupt_handler();

	msg_content_data_t content_storage;

	content_storage.data = malloc(options->message_size);
	bzero(content_storage.data, options->message_size);
	content_storage.capacity = options->message_size;
	content_storage.count = 0;
	content_storage.errors = 0;

	mpt_timestamp_t last;
	mpt_timestamp_t start = statistics_now();
	time_t last_calc = 0;

	statistics_latency_header(stat_io);
	statistics_throughput_header(stat_io);

	while (can_continue(options)) {
		vmsl->receive(msg_ctxt, &content_storage, &status);
		last = statistics_now();

		if (last_calc != last.tv_sec && (last.tv_sec % 10) == 0) {
			statistics_throughput_partial(stat_io, start, last, content_storage.count);

			last_calc = last.tv_sec;
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	statistics_destroy(&stat_io);

	uint64_t elapsed = statistics_diff(start, last);
	double rate = ((double) content_storage.count / elapsed) * 1000;

	uint64_t total_received = content_storage.count - 1;

	logger(STAT, "summary;received;%" PRIu64 ";elapsed;%" PRIu64 ";rate;%.2f",
		total_received, elapsed, rate);

	logger(INFO, "Summary: received %" PRIu64 " messages in %" PRIu64
				 " milliseconds (rate: %.2f msgs/sec)",
		total_received, elapsed, rate);
	logger(INFO, "Errors: received %" PRIu64, content_storage.errors);

	free(content_storage.data);
}
