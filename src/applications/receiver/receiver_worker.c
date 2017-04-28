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
	snprintf(name, len - 1, "%s-%d.csv.gz", prefix, getpid());
}

bool receiver_initialize_csv_writer(stats_writer_t *writer, const options_t *options, 
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

bool receiver_initialize_out_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	out_writer_latency_assign(&writer->latency);
	out_writer_throughput_assign(&writer->throughput);

	return true;
}


bool receiver_initialize_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	if (options->logdir) {
		return receiver_initialize_csv_writer(writer, options, status);
	}
	
	return receiver_initialize_out_writer(writer, options, status);	
}

static void receiver_print_partial(worker_info_t *worker_info) {
	worker_snapshot_t snapshot = {0};

	if (shr_buff_read(worker_info->shr, &snapshot, sizeof(worker_snapshot_t))) {
		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		printf("Partial summary: received %" PRIu64 " messages in %" PRIu64
				" seconds (rate: %.2f msgs/sec)",
				snapshot.count, elapsed, snapshot.throughput.rate);
	}
}

static void receiver_check_child(gru_list_t *list) {
	gru_node_t *node = NULL;

	if (list == NULL) {
		return;
	}

	node = list->root;

	uint32_t pos = 0;
	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);
		
		int wstatus = 0;
		pid_t pid = waitpid(worker_info->child, &wstatus, WNOHANG);

		// waitpid returns 0 if WNOHANG and there's no change of state for the process
		if (pid == 0) {
			receiver_print_partial(worker_info);

			node = node->next;
			pos++;
		}
		else {
			if (WIFEXITED(wstatus)) {
				printf("Child %d finished with status %d\n", 
					worker_info->child, WEXITSTATUS(wstatus));
			}
			else if (WIFSIGNALED(wstatus)) {
				printf("Child %d received a signal %d\n", 
					worker_info->child, WTERMSIG(wstatus));
			}
			else if (WIFSTOPPED(wstatus)) {
				printf("Child %d stopped %d\n", 
					worker_info->child, WSTOPSIG(wstatus));
			}

			gru_dealloc((worker_info_t **) &worker_info);
			
			node = node->next;
			pos++;
			gru_node_t *orphan = gru_list_remove(list, pos);
			
			gru_node_destroy(&orphan);
			
		}
	}
}

int receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	
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
	/*
	typedef struct worker_t_ {
	const vmsl_t *vmsl;
	const worker_options_t *options;
	const stats_writer_t *writer;
	worker_iteration_check can_continue;
} worker_t;
*/

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
	worker.name = "receiver";

	stats_writer_t writer = {0};
	worker.writer = &writer;
	receiver_initialize_writer(worker.writer, options, &status);

	worker.can_continue = worker_check;
	

	if (options->parallel_count == 1) { 
		worker_ret_t ret = {0}; 
		worker_snapshot_t snapshot = {0};

		ret = abstract_receiver_worker_start(&worker, &snapshot, &status);
		if (ret != WORKER_SUCCESS) {
			fprintf(stderr, "Unable to execute worker: %s\n", status.message);

			return 1;
		}

		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(INFO, 
			"Summary: received %" PRIu64 " messages in %" PRIu64
			" seconds (rate: %.2f msgs/sec)",
			snapshot.count, elapsed, snapshot.throughput.rate);
	}
	else {
		sleep(1);
		gru_list_t *children = abstract_worker_clone(&worker, 
			abstract_receiver_worker_start, &status);

		if (!children && !gru_status_success(&status)) {
			logger(ERROR, "Unable to initialize children: %s", status.message);

			return 1;
		}
		else {
			if (!children) {
				return 0;
			}
		}

		
		while (gru_list_count(children) > 0) {
			logger(INFO, "There are still %d children running", gru_list_count(children));
			receiver_check_child(children); 
			
			sleep(1);
		}

		gru_list_destroy(&children);
	}


	return 0;
}
