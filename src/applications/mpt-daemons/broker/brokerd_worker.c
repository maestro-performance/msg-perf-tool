/**
 *    Copyright 2017 Otavio Rodolfo Piske
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "brokerd_worker.h"

bool started = false;
bool halt = false;

worker_options_t worker_options = {0};
static char *locations[] = MAESTRO_BROKERI_DAEMON_SHEETS;

static void *brokerd_handle_set(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo)
{
	return commond_handle_set(request, response, &worker_options);
}

static void *brokerd_handle_flush(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	commond_handle_flush(request, response, pinfo);
	if (started) {
		gru_status_t status = gru_status_new();
		if (!bmic_writer_flush(&status)) {
			logger(GRU_ERROR, "Unable to flush bmic data to disk");
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}


static void *brokerd_handle_start(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a start request");
	if (started == true) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
	} else {
		started = true;

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	}

	return NULL;
}

static void *brokerd_handle_stop(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a stop request");
	started = false;

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *brokerd_handle_halt(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a halt request");
	brokerd_handle_stop(request, response, pinfo);
	halt = true;
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *brokerd_handle_stats(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static maestro_sheet_t *brokerd_new_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new(status);

	if (!ret) {
		return NULL;
	}

	maestro_sheet_set_location(ret, MAESTRO_DAEMON_SHEETS_COUNT, locations, MAESTRO_DEFAULT_QOS);

	maestro_instrument_t *start_instrument =
		maestro_instrument_new(MAESTRO_NOTE_START_INSPECTOR, brokerd_handle_start, status);

	maestro_sheet_add_instrument(ret, start_instrument);

	maestro_instrument_t *stop_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STOP_INSPECTOR, brokerd_handle_stop, status);

	maestro_sheet_add_instrument(ret, stop_instrument);

	maestro_instrument_t *flush_instrument =
		maestro_instrument_new(MAESTRO_NOTE_FLUSH, brokerd_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument =
		maestro_instrument_new(MAESTRO_NOTE_SET, brokerd_handle_set, status);

	maestro_sheet_add_instrument(ret, set_instrument);

	maestro_instrument_t *ping_instrument =
		maestro_instrument_new(MAESTRO_NOTE_PING, commond_handle_ping, status);

	maestro_sheet_add_instrument(ret, ping_instrument);

	maestro_instrument_t *stats_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STATS, brokerd_handle_stats, status);

	maestro_sheet_add_instrument(ret, stats_instrument);

	maestro_instrument_t *halt_instrument =
		maestro_instrument_new(MAESTRO_NOTE_HALT, brokerd_handle_halt, status);

	maestro_sheet_add_instrument(ret, halt_instrument);

	return ret;
}

static bool brokerd_abort_check(bmic_queue_stat_t *qstats) {
	if (worker_options.duration_type == MESSAGE_COUNT) {
		if ((qstats->msg_ack_count + qstats->msg_exp_count) >=
			worker_options.duration.count) {
			return true;
		}
	} else {
		gru_timestamp_t now = gru_time_now();

		if (now.tv_sec >= worker_options.duration.time.end.tv_sec) {
			return true;
		}
	}

	return false;
}



static void bmic_dump_data(FILE *file, void *data) {
	bmic_stats_set_t *bmic_wrapper = (bmic_stats_set_t *) data;

	gru_config_write_string("jvmName", file, bmic_wrapper->java_info.name);
	gru_config_write_string("jvmVersion", file, bmic_wrapper->java_info.version);
	gru_config_write_string("jvmPackageVersion", file, bmic_wrapper->java_info.jvm_package_version);
	gru_config_write_string("operatingSystemName", file, bmic_wrapper->os_info.name);
	gru_config_write_string("operatingSystemArch", file, bmic_wrapper->os_info.arch);
	gru_config_write_string("operatingSystemVersion", file, bmic_wrapper->os_info.version);
	gru_config_write_long("systemCpuCount", file, bmic_wrapper->os_info.cpus);
	gru_config_write_long("systemMemory", file, bmic_wrapper->os_info.mem_total);
	gru_config_write_long("systemSwap", file, bmic_wrapper->os_info.swap_total);

	gru_config_write_string("productName", file, bmic_wrapper->product_info->name);
	gru_config_write_string("productVersion", file, bmic_wrapper->product_info->version);

	fflush(file);
}


static bool brokerd_dump(const char *dir, bmic_stats_set_t *bmic_data, gru_status_t *status) {
	gru_config_t *config = gru_config_new(dir, "broker.properties", status);
	gru_alloc_check(config, false);

	gru_payload_t *payload = gru_payload_init(NULL, bmic_dump_data, NULL, bmic_data, status);
	if (!payload) {
		gru_config_destroy(&config);
		return false;
	}

	if (!gru_config_init_for_dump(config, payload, status)) {

		gru_payload_destroy(&payload);
		gru_config_destroy(&config);
		return false;
	}

	gru_payload_destroy(&payload);
	gru_config_destroy(&config);

	return true;
}

static bool brokerd_update(const bmic_context_t *ctxt, bmic_stats_set_t *stats, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	bmic_api_interface_t *api = ctxt->api;

	if (!bmic_writer_current_time(status)) {
		logger(GRU_ERROR, "Unable to write current time: %s", status->message);

		return false;
	}

	stats->os_info = api->java.os_info(ctxt->handle, status);

	bmic_writer_osinfo(&stats->os_info);

	mpt_get_mem_info(ctxt, stats->java_info.memory_model, &stats->java_mem, status);
	if (gru_status_error(status)) {
		logger(GRU_ERROR, "%s", status->message);
		return false;
	}

	bmic_writer_java_mem(&stats->java_mem, stats->java_info.memory_model);


	mpt_get_queue_stats(ctxt, &worker_options.uri.path[1], &stats->queue_stats, status);

	if (gru_status_error(status)) {
		logger(GRU_ERROR, "%s", status->message);
		return false;
	}
	bmic_writer_queue_stat(&stats->queue_stats);

	return true;
}

static void brokerd_clean_stats_set(bmic_stats_set_t *stats_set) {
	if (!stats_set) {
		return;
	}

	bmic_product_info_cleanup(&stats_set->product_info);
	bmic_java_info_cleanup(stats_set->java_info);
}

static bool brokerd_collect(gru_status_t *status) {
	logger_t logger = gru_logger_get();
	bmic_context_t ctxt = {0};
	bmic_stats_set_t stats_set = {0};

	const char filename[64] = "broker-jvm-inspector.csv.gz";

	char worker_log_dir[PATH_MAX] = {0};
	if (!worker_log_init(worker_log_dir, status)) {
		return WORKER_FAILURE;
	}

	logger(GRU_INFO, "Initializing BMIC writer");
	if (!bmic_writer_initialize(worker_log_dir, filename, status)) {
		goto err_exit;
	}

	logger(GRU_INFO, "Initializing BMIC context");
	if (!mpt_init_bmic_ctxt(worker_options.uri, &ctxt, status)) {
		goto err_exit;
	}

	logger(GRU_INFO, "Purging the queue and resetting the counters");
	if (!mpt_purge_queue(&ctxt, &worker_options.uri.path[1], status)) {
		goto err_exit;
	}

	logger(GRU_INFO, "Reading broker properties");
	bmic_api_interface_t *api = ctxt.api;

	const bmic_exchange_t *cap = api->capabilities_load(ctxt.handle, status);
	if (!cap) {
		bmic_context_cleanup(&ctxt);
		goto err_exit;
	}

	stats_set.product_info = api->product_info(ctxt.handle, cap, status);
	stats_set.java_info = api->java.java_info(ctxt.handle, status);
	stats_set.os_info = api->java.os_info(ctxt.handle, status);

	logger(GRU_INFO, "Writing broker properties");
	if (!brokerd_dump(worker_log_dir, &stats_set, status)) {
		goto err_exit;
	}

	if (!worker_dump(worker_log_dir, &worker_options, status)) {
		goto err_exit;
	}

	while (started) {
		if (!brokerd_update(&ctxt, &stats_set, status)) {
			break;
		}

		if (brokerd_abort_check(&stats_set.queue_stats)) {
			break;
		}

		bmic_writer_flush(status);
		sleep(MPT_BROKERD_COLLECT_WAIT_TIME);
	}

	if (!gru_status_success(status)) {
		bmic_writer_finalize(status);
		bmic_context_cleanup(&ctxt);
		logger(GRU_INFO, "Broker inspector completed the inspection with errors: %s",
			   status->message);

		worker_log_link_create(worker_log_dir, options_get_log_dir(), "lastFailed");
		maestro_notify_test_failed(status);

		return false;
	}

	brokerd_clean_stats_set(&stats_set);

	worker_log_link_create(worker_log_dir, options_get_log_dir(), "last");
	worker_log_link_create(worker_log_dir, options_get_log_dir(), "lastSuccessful");

	maestro_notify_test_successful(status);

	bmic_writer_finalize(status);
	bmic_context_cleanup(&ctxt);
	logger(GRU_INFO, "Broker inspector completed the inspection");

	return true;

	err_exit:
	brokerd_clean_stats_set(&stats_set);

	worker_log_link_create(worker_log_dir, options_get_log_dir(), "last");
	worker_log_link_create(worker_log_dir, options_get_log_dir(), "lastFailed");

	maestro_notify_test_failed(status);

	bmic_writer_finalize(status);
	bmic_context_cleanup(&ctxt);
	logger(GRU_INFO, "Broker inspector completed the inspection with errors");
	return false;
}

int brokerd_worker_start(const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	maestro_sheet_t *sheet = brokerd_new_sheet(&status);

	if (!maestro_player_start(options, sheet, &status)) {
		logger(GRU_FATAL, "Unable to connect to maestro broker: %s", status.message);

		maestro_player_stop(sheet, &status);
		maestro_sheet_destroy(&sheet);
		return 1;
	}

	while (!halt) {
		usleep(MPT_WORKER_IDLE_TIME);

		if (started) {
			if (!brokerd_collect(&status)) {
				logger(GRU_ERROR, "Unable to collect broker data: %s", status.message);
			}

			started = false;
		}

		fflush(NULL);
	}

	maestro_player_stop(sheet, &status);
	maestro_sheet_destroy(&sheet);

	return 0;
}