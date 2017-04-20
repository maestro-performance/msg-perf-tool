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
#include "receiver_worker.h"
#include "vmsl.h"

bool can_start = false;

static void *receiver_handle_start(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	can_start = true;
	return NULL;
}

static maestro_sheet_t *new_receiver_sheet(gru_status_t *status) {
	//char *location = NULL;
	//asprintf(&location, "/mpt/receiver/%d", getpid());

	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);
	// gru_dealloc_string(&location);

	if (!ret) {	
		return NULL;
	}

	maestro_instrument_t *instrument = maestro_instrument_new(MAESTRO_NOTE_START, 
		receiver_handle_start, status);

	maestro_sheet_add_instrument(ret, instrument);

	return ret;
}


void receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	const uint32_t tp_interval = 10;
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0}; 

	// maestro_sheet_t *sheet = new_receiver_sheet(&status);

	// if (!maestro_player_start(options, sheet, &status)) {
	// 	fprintf(stderr, "Unable to connect to maestro broker: %s\n", 
	// 		status.message);

	// 	return;
	// }

	// while (!can_start) {
	// 	logger(INFO, "Waiting for the start signal");
	// 	sleep(1);
	// }

	stat_io_t *stat_io = statistics_init(RECEIVER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);
		gru_status_reset(&status);

		return;
	}

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_RECEIVER, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_DEFAULT,
	};

	msg_conn_info_gen_id(&opt.conn_info);
	opt.uri = options->uri;

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, opt, NULL, &status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	vmsl_stat_t ret = vmsl->subscribe(msg_ctxt, NULL, &status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	msg_content_data_init(&content_storage, options->message_size, &status);
	if (!gru_status_success(&status)) {
		goto err_exit;
	}

	gru_timestamp_t last;
	gru_timestamp_t start = gru_time_now();
	time_t last_calc = 0;

	statistics_latency_header(stat_io);
	statistics_throughput_header(stat_io);

	// install_timer(30);
	install_interrupt_handler();

	register uint64_t count = 0;

	while (can_continue(options, 0)) {
		vmsl_stat_t rstat = vmsl->receive(msg_ctxt, &content_storage, &status);
		if (unlikely(vmsl_stat_error(rstat))) {
			logger(ERROR, "Error receiving data: %s\n", status.message);

			gru_status_reset(&status);
			break;
		}

		if (rstat & VMSL_NO_DATA) {
			continue;
		}

		count++;

		last = gru_time_now();

		statistics_latency(stat_io, content_storage.created, last);

		if (last_calc <= (last.tv_sec - tp_interval)) {
			uint64_t processed_count = count - last_count;

			statistics_throughput_partial(stat_io, last, tp_interval, processed_count);
			
			last_count = count;
			last_calc = last.tv_sec;
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	statistics_destroy(&stat_io);

	uint64_t elapsed = statistics_diff(start, last);
	double rate = ((double) count / (double) elapsed) * 1000;

	uint64_t total_received = count;

	logger(STAT,
		"summary;received;%" PRIu64 ";elapsed;%" PRIu64 ";rate;%.2f",
		total_received,
		elapsed,
		rate);

	logger(INFO,
		"Summary: received %" PRIu64 " messages in %" PRIu64
		" milliseconds (rate: %.2f msgs/sec)",
		total_received,
		elapsed,
		rate);

	msg_content_data_release(&content_storage);
	return;

err_exit:
	fprintf(stderr, "%s", status.message);
	statistics_destroy(&stat_io);
	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		vmsl->destroy(msg_ctxt, &status);
	}

	gru_status_reset(&status);
	return;
}
