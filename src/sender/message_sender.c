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

static bool interrupted = false;
static char *data = NULL;
static size_t capacity;

static void timer_handler(int signum) {
	// NO-OP for now
}

static void interrupt_handler(int signum) { interrupted = true; }

static void install_timer() {
	struct sigaction sa;
	struct itimerval timer;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &timer_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGALRM, &sa, NULL);

	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;

	timer.it_interval.tv_sec = 1;
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

static bool can_continue(const options_t *options, uint64_t sent) {
	struct timeval now;

	if (interrupted) {
		return false;
	}

	gettimeofday(&now, NULL);

	if ((sent < options->count) || options->count == 0) {
		if (now.tv_sec <= options->duration.end.tv_sec ||
			options->duration.end.tv_sec == 0) {
			return true;
		}
	}

	return false;
}

static const char *load_message_data(const options_t *options) {
	data = malloc(options->message_size + 1);

	logger_t logger = gru_logger_get();
	if (data == NULL) {

		logger(FATAL, "Unable to allocate memory for the message data");

		return NULL;
	}

	logger(INFO, "Loading %d bytes for message data", options->message_size);

	bzero(data, options->message_size);

	for (size_t i = 0; i < options->message_size; i++) {
		data[i] = 'c';
	}

	data[options->message_size] = 0;
	capacity = options->message_size;
	return data;
}

static void unload_message_data() {
	free(data);
	capacity = 0;
}

static void content_loader(msg_content_data_t *content_data) {
	content_data->capacity = capacity;
	content_data->size = capacity;
	content_data->data = data;
}

void sender_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = {0};

	stat_io_t *stat_io = statistics_init(SENDER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);

		return;
	}
	logger(TRACE, "Initializing test execution");

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, NULL);

	install_timer();
	install_interrupt_handler();
	load_message_data(options);

	mpt_timestamp_t last;
	mpt_timestamp_t start = statistics_now();

	register uint64_t sent = 0;
	time_t last_calc = 0;

	statistics_throughput_header(stat_io);
	while (can_continue(options, sent)) {
		vmsl->send(msg_ctxt, content_loader);
		sent++;
		last = statistics_now();

		if (last_calc != last.tv_sec && (last.tv_sec % 10) == 0) {
			statistics_throughput_partial(stat_io, start, last, sent);

			last_calc = last.tv_sec;
		}

		if (options->throttle > 0) {
			if (((sent % options->throttle) == 0)) {
				usleep(1000000 - last.tv_usec);
			}
		}
	}

	vmsl->stop(msg_ctxt);
	vmsl->destroy(msg_ctxt);

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