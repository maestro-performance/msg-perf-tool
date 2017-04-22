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
	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);
	
	if (!ret) {	
		return NULL;
	}

	maestro_instrument_t *instrument = maestro_instrument_new(MAESTRO_NOTE_START, 
		receiver_handle_start, status);

	maestro_sheet_add_instrument(ret, instrument);

	return ret;
}

static void receiver_csv_name(const char *prefix, char *name, size_t len) 
{
	snprintf(name, len - 1, "%s-%d.csv", prefix, getpid());
}

bool receiver_initialize_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	csv_writer_latency_assign(&writer->latency);
	csv_writer_throughput_assign(&writer->throughput);

	char lat_fname[64] = {0};
	receiver_csv_name("receiver-latency", lat_fname, sizeof(lat_fname));

	stat_io_info_t lat_io_info = {0};
	lat_io_info.dest.name = lat_fname;
	lat_io_info.dest.location = (char *) options->logdir;

	if (!writer->latency.initialize(&lat_io_info, status)) {
		return false;
	}

	char tp_fname[64] = {0};
	receiver_csv_name("receiver-throughput", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) options->logdir;

	if (!writer->throughput.initialize(&tp_io_info, status)) {
		return false;
	}
	
	return true;
}


void receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	const uint32_t sample_interval = 10; // sampling interval
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

	// stat_io_t *stat_io = statistics_init(RECEIVER, &status);
	// if (!stat_io) {
	// 	logger(FATAL, "Unable to initialize statistics engine: %s", status.message);
	// 	gru_status_reset(&status);

	// 	return;
	// }

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_RECEIVER, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_DEFAULT,
	};

	msg_conn_info_gen_id(&opt.conn_info);
	opt.uri = options->uri;

	msg_ctxt_t *msg_ctxt = vmsl->init(opt, NULL, &status);
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

	stats_writer_t writer = {0};
	if (!receiver_initialize_writer(&writer, options, &status)) {
		logger(ERROR, "Unable to initialize writer: %s", status.message);
		
		goto err_exit;
	}

	gru_timestamp_t start = gru_time_now();
	gru_timestamp_t now = start;
	gru_timestamp_t last_sample_ts = start; // Last sampling timestamp

	install_interrupt_handler();

	register uint64_t count = 0;
	stat_latency_t lat_out = {0};
	stat_throughput_t tp_out = {0};

	while (can_continue(options, 0)) {
		vmsl_stat_t rstat = vmsl->receive(msg_ctxt, &content_storage, &status);
		if (unlikely(vmsl_stat_error(rstat))) {
			logger(ERROR, "Error receiving data: %s\n", status.message);

			gru_status_reset(&status);
			break;
		}

		if (rstat & VMSL_NO_DATA) {
			usleep(500);
			continue;
		}

		count++;

		now = gru_time_now();

		
		calc_latency(&lat_out, content_storage.created, now);
		if (unlikely(!writer.latency.write(&lat_out, &status))) {
			logger(ERROR, "Unable to write latency data: %s", status.message);

			gru_status_reset(&status);
			break;
		}
		

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
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	writer.latency.finalize(&status);
	writer.throughput.finalize(&status);

	uint64_t elapsed = gru_time_elapsed_secs(start, now);
	calc_throughput(&tp_out, start, now, count);

	logger(INFO, 
		"Summary: received %" PRIu64 " messages in %" PRIu64
		" seconds (rate: %.2f msgs/sec)",
		tp_out.count, elapsed, tp_out.rate);

	msg_content_data_release(&content_storage);
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
